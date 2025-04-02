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

    let btdMax = 0; // Max BT/D of all files used for drawing the Y axis
    let decisionsMax = 0; // Max of total decisions of all files used for drawing the X axis

    // Variables count in file
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
        btdMax = 0;
        decisionsMax = 0;
        decisionsInFile = 0;
        backtracksInFile = 0;
        currentGroupIds = [];
        dataPairsGroups = {};
    }

    /**
     * Draws chart which is a scatter plot of from overall BT/D and total decisions per file.
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
                "btd",
                decisionsMax,
                btdMax,
                4,
                getColorForGroup(groupId),
            );
        }

        drawAxes(margin, W, H);
        drawTitle(
            ctx,
            W,
            margin,
            "Total Decisions vs Overall Backtracks per Decision",
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
            "Overall Backtracks per Decision",
        );
        drawTicksX(ctx, margin, W, H, decisionsMax, 10);
        drawTicksY(ctx, margin, W, H, btdMax, 10);
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
     * Finds a number total decisions and total backtracks in the file and stores them global variables
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
     * Callback function to parse a single CSV file.
     * Creates data pairs of total decisions and BT/D per file, puts them into right group and computes maximum values for the axes.
     */
    function onFileDone() {
        // Show only files where at least one decision was made
        if (decisionsInFile > 0) {
            const btd = backtracksInFile / decisionsInFile;

            for (const groupId of currentGroupIds) {
                // Data does not exist yet
                if (!dataPairsGroups[groupId]) dataPairsGroups[groupId] = [];

                // Store in appropriate group
                dataPairsGroups[groupId].push({
                    decisions: decisionsInFile,
                    btd: btd,
                });
            }

            decisionsMax = Math.max(decisionsMax, decisionsInFile);
            btdMax = Math.max(btdMax, btd);
        }
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
