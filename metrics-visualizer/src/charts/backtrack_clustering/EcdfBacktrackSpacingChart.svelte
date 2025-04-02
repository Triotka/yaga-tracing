<script>
    import { onMount, onDestroy } from "svelte";
    import { subscribe } from "../../lib/parser.js";
    import {
        drawEmptyLabeledAxes,
        initCanvas,
        drawTitle,
        drawEcdfLineFromHistogram,
        drawTicksX,
        drawTicksY,
        getColorForGroup,
    } from "../../lib/canvas.js";

    // CONSTANTS
    const NUM_BINS = 50; // Number of bins in histogram
    const MAX_GAP = 50; // A decision gap between backtracks that is bigger than this constant fall into a last bin

    let canvas, ctx;

    // Variables count in file
    let previousDecisionCount = 0;
    let firstBacktrack = true;
    let currentGroupIds = [];

    let histograms = {}; // Maps groupId to its histogram
    let totalGapsCounts = {}; // Maps groupId to total gaps for this group

    let unsubscribe; // Stores the unsubscribe function

    /**
     * Restarts data holders for the next chart data.
     *
     */
    function restartDataHolders() {
        previousDecisionCount = 0;
        firstBacktrack = true;
        currentGroupIds = [];
        histograms = {};
        totalGapsCounts = {};
    }

    /**
     * Callback function to parse a single CSV file.
     * Stores current file's group IDs in the global variable.
     */
    async function onFileStart(file, groupIds) {
        currentGroupIds = groupIds;
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
        drawEmptyLabeledAxes(ctx, W, H, margin, "Gap Size", "ECDF");
        drawTicksX(ctx, margin, W, H, MAX_GAP, 10);
        drawTicksY(ctx, margin, W, H, 1, 10);
    }

    /**
     * Draws chart with ECDFs of Backtrack Spacing.
     */
    function drawChart() {
        const margin = { top: 40, right: 40, bottom: 60, left: 60 };
        const W = canvas.width;
        const H = canvas.height;

        ctx.clearRect(0, 0, W, H);
        // Iterate over all existing groups and create ECDF line for each one
        for (const groupId in histograms) {
            const histogram = histograms[groupId];
            const totalGaps = totalGapsCounts[groupId];
            drawEcdfLineFromHistogram(
                ctx,
                margin,
                W,
                H,
                histogram,
                NUM_BINS,
                MAX_GAP,
                totalGaps,
                getColorForGroup(groupId),
            );
        }

        drawAxes(margin, W, H);
        drawTitle(
            ctx,
            W,
            margin,
            "Empirical Cumulative Distribution Function of Backtrack Spacing",
        );
    }
    /**
     * Callback function to process a single CSV row.
     * Calculates decision gap and increments appropriate histograms.
     *
     * @param {Array} row - A single row from the CSV.
     */
    function onRow(row) {
        const origin = row[0];
        const event = row[1];
        if (event === "before_backtrack" && origin === "core") {
            const currentDecisionCount = +row[4];
            if (!firstBacktrack) {
                const gap = currentDecisionCount - previousDecisionCount;
                const index = Math.min(
                    Math.floor((gap / MAX_GAP) * NUM_BINS),
                    NUM_BINS - 1,
                );
                for (const groupId of currentGroupIds) {
                    // Data does not exist yet
                    if (!histograms[groupId])
                        histograms[groupId] = new Array(NUM_BINS).fill(0);
                    if (!totalGapsCounts[groupId]) totalGapsCounts[groupId] = 0;

                    // Store in appropriate histogram
                    histograms[groupId][index]++;
                    totalGapsCounts[groupId]++;
                }
            }
            previousDecisionCount = currentDecisionCount;
            firstBacktrack = false;
        }
    }

    /**
     * Callback function to parse a single CSV file.
     * Restarts file count variables
     */
    function onFileDone() {
        previousDecisionCount = 0;
        firstBacktrack = true;
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
