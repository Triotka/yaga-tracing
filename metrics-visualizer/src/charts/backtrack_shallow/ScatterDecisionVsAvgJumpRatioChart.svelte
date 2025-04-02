<script>
    import { onMount, onDestroy } from "svelte";
    import { subscribe } from "../../lib/parser.js";
    import {
        drawEmptyLabeledAxes,
        initCanvas,
        drawTitle,
        drawTicksX,
        drawTicksY,
        drawScatterDataPoints,
        getColorForGroup,
    } from "../../lib/canvas.js";

    let canvas, ctx;

    let avgJumpRatioMax = 0; // Max average jump ratio out of all files used for drawing the Y axis
    let decisionsMax = 0; // Max of total decisions of all files used for drawing the X axis

    // Variables used for parsing
    let decisionsInFile = 0;
    let sumJumpRatiosInFile = 0;
    let countJumpRatiosInFile = 0;
    let trailSizeBefore = 0;
    let currentGroupIds = [];

    let dataPairsGroups = {}; // Maps groupId to its data pairs

    let unsubscribe; // Stores the unsubscribe function

    /**
     * Restarts data holders for the next chart data.
     *
     */
    function restartDataHolders() {
        avgJumpRatioMax = 0;
        decisionsMax = 0;
        decisionsInFile = 0;
        sumJumpRatiosInFile = 0;
        countJumpRatiosInFile = 0;
        trailSizeBefore = 0;
        currentGroupIds = [];
        dataPairsGroups = {};
    }

    /**
     * Draws chart which is a scatter plot of from average jump ratio and total decisions per file.
     */
    function drawChart() {
        const margin = { top: 40, right: 40, bottom: 60, left: 60 };
        const W = canvas.width;
        const H = canvas.height;
        ctx.clearRect(0, 0, W, H);

        // Iterate over all existing groups and plot points for each one
        for (const groupId in dataPairsGroups) {
            const dataPairs = dataPairsGroups[groupId];
            drawScatterDataPoints(
                ctx,
                dataPairs,
                margin,
                W,
                H,
                "decisions",
                "avgJumpRatio",
                decisionsMax,
                avgJumpRatioMax,
                4,
                getColorForGroup(groupId),
            );
        }
        drawAxes(margin, W, H);
        drawTitle(
            ctx,
            W,
            margin,
            "Total Decisions vs. Average Backtrack Jump Ratio",
        );
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
            "Total Decisions",
            "Average Jump Ratio",
        );
        drawTicksX(ctx, margin, W, H, decisionsMax, 10);
        drawTicksY(ctx, margin, W, H, avgJumpRatioMax, 10);
    }

    /**
     * Callback function to parse a single CSV file.
     * Stores current file's group IDs in the global variable.
     */
    async function onFileStart(file, groupIds) {
        currentGroupIds = groupIds;
    }

    /**
     * Callback function to process a single CSV row.
     * Finds a number total decisions, sum of jump ratios and jump ratios count in the file and stores them in global variables
     *
     * @param {Array} row - A single row from the CSV.
     */
    function onRow(row) {
        const origin = row[0];
        const event = row[1];

        if (origin === "core" && event === "search_end") {
            decisionsInFile = +row[7];
        } else if (event === "before_backtrack" && origin == "core") {
            trailSizeBefore = +row[3];
        } else if (event === "after_backtrack" && origin == "core") {
            const trailSizeAfter = +row[3];
            if (trailSizeBefore > 0) {
                const jumpRatio =
                    (trailSizeBefore - trailSizeAfter) / trailSizeBefore;
                sumJumpRatiosInFile += jumpRatio;
                countJumpRatiosInFile++;
            } else {
                console.warn("backtracking from trail size 0");
            }
        }
    }

    /**
     * Callback function to parse a single CSV file.
     * Creates data pairs of total decisions and jump ratio average per file and computes maximum values for the axes.
     */
    function onFileDone() {
        // Show only files where at least one jump ratio occured
        if (countJumpRatiosInFile > 0) {
            const avgJumpRatioInFile =
                sumJumpRatiosInFile / countJumpRatiosInFile;

            for (const groupId of currentGroupIds) {
                // Data does not exist yet
                if (!dataPairsGroups[groupId]) dataPairsGroups[groupId] = [];

                // Store in appropriate group
                dataPairsGroups[groupId].push({
                    decisions: decisionsInFile,
                    avgJumpRatio: avgJumpRatioInFile,
                });
            }
            decisionsMax = Math.max(decisionsMax, decisionsInFile);
            avgJumpRatioMax = Math.max(avgJumpRatioMax, avgJumpRatioInFile);
        }
        // Reset for the next file
        decisionsInFile = 0;
        sumJumpRatiosInFile = 0;
        countJumpRatiosInFile = 0;
        trailSizeBefore = 0;
    }

    /**
     * Callback function called after all CSV fileshave been parsed.
     * Draws a chart and restarts data holders.
     */
    function onAllDone() {
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
