<script>
    import { onMount, onDestroy } from "svelte";
    import { subscribe, readParsedLastLine } from "../../lib/parser.js";
    import {
        drawEmptyLabeledAxes,
        initCanvas,
        drawTitle,
        drawTicksX,
        drawTicksY,
        drawLineFromBins,
        getColorForGroup,
    } from "../../lib/canvas.js";

    // CONSTANTS
    const NUM_BINS = 50; // number of bins in histogram
    const MAX_JUMP_RATIO = 50; // a jump ration that is bigger than this constant fall into a last bin

    let canvas, ctx;
    let unsubscribe; // Stores the unsubscribe function

    let jumpRatioSumBinsByGroup = {}; // Maps groupId to its bin with jump ratio sum
    let jumpRatioCountBinsByGroup = {}; // Maps groupId to its bin with jump ratio count

    let maxAverageJumRatio = 0;

    // Count variables
    let normalizedProgress = 0;
    let maxProgressPerFile = 0;
    let trailSizeBefore = 0;
    let currentGroupIds = [];

    /**
     * Restarts data holders for the next chart data.
     *
     */
    function restartDataHolders() {
        jumpRatioSumBinsByGroup = {};
        jumpRatioCountBinsByGroup = {};
        maxAverageJumRatio = 0;
        normalizedProgress = 0;
        maxProgressPerFile = 0;
        trailSizeBefore = 0;
        currentGroupIds = [];
    }

    /**
     * Draws the chart axes along with labeled tick marks on both X and Y axes.
     *
     * @param {Object} margin - Margins of the chart {top, right, bottom, left}.
     * @param {number} W - The total width of the canvas.
     * @param {number} H - The total height of the canvas.
     *
     */
    function drawAxes(margin, W, H) {
        drawEmptyLabeledAxes(
            ctx,
            W,
            H,
            margin,
            "Normalized progress",
            "Average Jump Ratio",
        );
        drawTicksX(ctx, margin, W, H, 1, 10);
        drawTicksY(ctx, margin, W, H, maxAverageJumRatio, 10);
    }

    /**
     * Calculates data's max jump ratio value of average jump ration and stores them in global valus.
     */
    function calculateMaxJumpRatio() {
        for (const groupId in jumpRatioSumBinsByGroup) {
            for (let i = 0; i < jumpRatioSumBinsByGroup[groupId].length; i++) {
                if (jumpRatioCountBinsByGroup[groupId][i] === 0) continue;
                const avg =
                    jumpRatioSumBinsByGroup[groupId][i] /
                    jumpRatioCountBinsByGroup[groupId][i];
                const cappedAverage = Math.min(avg, MAX_JUMP_RATIO);
                maxAverageJumRatio = Math.max(
                    maxAverageJumRatio,
                    cappedAverage,
                );
            }
        }
    }

    /**
     * Draws a chart.
     */
    function drawChart() {
        const margin = { top: 40, right: 40, bottom: 60, left: 60 };
        const W = canvas.width;
        const H = canvas.height;
        ctx.clearRect(0, 0, W, H);

        // Iterate over all existing groups and create line for each one
        for (const groupId in jumpRatioSumBinsByGroup) {
            const jumpRatioSumBins = jumpRatioSumBinsByGroup[groupId];
            const jumpRatioCountBins = jumpRatioCountBinsByGroup[groupId];
            drawLineFromBins(
                ctx,
                margin,
                W,
                H,
                jumpRatioSumBins,
                jumpRatioCountBins,
                NUM_BINS,
                MAX_JUMP_RATIO,
                maxAverageJumRatio,
                getColorForGroup(groupId),
            );
        }
        drawAxes(margin, W, H);
        drawTitle(
            ctx,
            W,
            margin,
            "Normalized Progress Vs Backtrack Jump ratio Average",
        );
    }

    /**
     * Finds total decision in last line of the log file and updates maxProgressInFile with it.
     * @param {File} file - Log file to search in.
     */
    async function setMaxProgressInFile(file) {
        const row = await readParsedLastLine(file);
        const origin = row[0];
        const event = row[1];

        if (event === "search_end" && origin === "core") {
            maxProgressPerFile = +row[7];
        } else {
            console.warn("Max progress not present");
            return;
        }
    }

    /**
     * Callback function to process a single CSV row.
     * Calculates backtrack jump ratio and increments bins.
     *
     * @param {Array} row - A single row from the CSV.
     */
    function onRow(row) {
        const origin = row[0];
        const event = row[1];

        if (maxProgressPerFile === null || maxProgressPerFile === 0) {
            console.warn("invalid max progress");
            return;
        }
        if (event === "before_backtrack" && origin === "core") {
            trailSizeBefore = +row[3];
            const currentProgress = +row[4];
            normalizedProgress = currentProgress / maxProgressPerFile;
        } else if (event === "after_backtrack" && origin === "core") {
            const trailSizeAfter = +row[3];
            if (trailSizeBefore > 0) {
                const jumpRatio =
                    (trailSizeBefore - trailSizeAfter) / trailSizeBefore;

                // Stores in bins
                const index = Math.min(
                    Math.floor(normalizedProgress * NUM_BINS),
                    NUM_BINS - 1,
                );
                for (const groupId of currentGroupIds) {
                    // Data does not exist yet
                    if (!jumpRatioSumBinsByGroup[groupId])
                        jumpRatioSumBinsByGroup[groupId] = new Array(
                            NUM_BINS,
                        ).fill(0);
                    if (!jumpRatioCountBinsByGroup[groupId])
                        jumpRatioCountBinsByGroup[groupId] = new Array(
                            NUM_BINS,
                        ).fill(0);

                    jumpRatioSumBinsByGroup[groupId][index] += jumpRatio;
                    jumpRatioCountBinsByGroup[groupId][index]++;
                }
            }
        }
    }

    /**
     * Callback function to parse a single CSV file.
     * Sets maxProgressPerFile from the file's last line.
     */
    async function onFileStart(file, groupIds) {
        currentGroupIds = groupIds;
        await setMaxProgressInFile(file);
    }
    /**
     * Callback function to parse a single CSV file.
     * Restarts count variables.
     */
    function onFileDone() {
        normalizedProgress = 0;
        maxProgressPerFile = 0;
        trailSizeBefore = 0;
    }

    /**
     * Callback function called after all CSV fileshave been parsed.
     * Calculates max jump ratio, draws a chart and restarts data holders.
     */
    function onAllDone() {
        calculateMaxJumpRatio();
        drawChart();
        restartDataHolders();
    }

    onDestroy(() => {
        unsubscribe && unsubscribe(); // Clean up
    });

    onMount(async () => {
        ctx = initCanvas(canvas);
        unsubscribe = subscribe(onRow, onFileDone, onAllDone, onFileStart);
    });
</script>

<div class="chart-wrapper">
    <canvas bind:this={canvas}></canvas>
</div>

<style>
    .chart-wrapper {
        text-align: center;
        background: #fff;
        padding: 10px;
    }

    canvas {
        width: 100% !important;
        height: auto !important;
        display: block;
        margin: 0 auto;
        border: 1px solid #ccc;
    }
</style>
