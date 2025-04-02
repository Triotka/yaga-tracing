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

    // CONSTANTS
    const WINDOW_SIZE = 20; // Fixed decision window size

    let canvas, ctx;

    let avgClusterSizeMax = 0; // Max average cluster size out of all files used for drawing the Y axis
    let decisionsMax = 0; // Max of total decisions of all files used for drawing the X axis

    // Variables used for parsing
    let decisionsInFile = 0;
    let backtracksInFile = 0;
    let currentGroupIds = [];

    let dataPairsGroups = {}; // Maps groupId to its data pairs

    let unsubscribe; // Stores the unsubscribe function

    /**
     * Restarts data holders for the next chart data.
     *
     */
    function restartDataHolders() {
        avgClusterSizeMax = 0;
        decisionsMax = 0;
        decisionsInFile = 0;
        backtracksInFile = 0;
        currentGroupIds = [];
        dataPairsGroups = {};
    }

    /**
     * Draws chart which is a scatter plot of from average cluster size and total decisions per file.
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
                "avgClusterSize",
                decisionsMax,
                avgClusterSizeMax,
                4,
                getColorForGroup(groupId),
            );
        }
        drawAxes(margin, W, H);
        drawTitle(
            ctx,
            W,
            margin,
            "Total Decisions vs. Average Backtrack Cluster Size",
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
            "Average Cluster Size",
        );
        drawTicksX(ctx, margin, W, H, decisionsMax, 10);
        drawTicksY(ctx, margin, W, H, avgClusterSizeMax, 10);
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
     * Finds a number total decisions and total backtracks in the file and stores them in global variables
     *
     * @param {Array} row - A single row from the CSV.
     */
    function onRow(row) {
        const origin = row[0];
        const event = row[1];

        if (origin === "core" && event === "search_end") {
            decisionsInFile = +row[7];
            backtracksInFile = +row[8];
        }
    }
    /**
     * Calculates average cluster size from total decisions and total backtracks in file.
     * @returns {number} Average cluster size.
     */
    function calculateAvgClusterSize() {
        const fullWindows = Math.floor(decisionsInFile / WINDOW_SIZE);
        const tailSize = decisionsInFile % WINDOW_SIZE;
        const effectiveWindows = fullWindows + tailSize / WINDOW_SIZE;
        return effectiveWindows > 0 ? backtracksInFile / effectiveWindows : 0;
    }

    /**
     * Callback function to parse a single CSV file.
     * Creates data pairs of total decisions and average cluster size per file and computes maximum values for the axes.
     */
    function onFileDone() {
        const avgClusterSizeInFile = calculateAvgClusterSize();
        for (const groupId of currentGroupIds) {
            // Data does not exist yet
            if (!dataPairsGroups[groupId]) dataPairsGroups[groupId] = [];

            // Store in appropriate group
            dataPairsGroups[groupId].push({
                decisions: decisionsInFile,
                avgClusterSize: avgClusterSizeInFile,
            });
        }

        decisionsMax = Math.max(decisionsMax, decisionsInFile);
        avgClusterSizeMax = Math.max(avgClusterSizeMax, avgClusterSizeInFile);

        // Reset for the next file
        decisionsInFile = 0;
        backtracksInFile = 0;
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
