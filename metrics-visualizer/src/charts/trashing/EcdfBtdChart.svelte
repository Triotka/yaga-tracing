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
    const NUM_BINS = 50; // number of bins in histogram
    const MAX_BTD = 2.0; // all files that have BT/D more that this constant fall into a last bin

    let canvas, ctx;

    // Variables count in file
    let decisionsInFile = 0;
    let backtracksInFile = 0;
    let currentGroupIds = [];

    let histograms = {}; // Maps groupId to its histogram
    let processedFilesCounts = {}; // Maps groupId to total processed files for this group

    let unsubscribe; // Stores the unsubscribe function

    /**
     * Restarts data holders for the next chart data.
     *
     */
    function restartDataHolders() {
        decisionsInFile = 0;
        backtracksInFile = 0;
        currentGroupIds = [];
        histograms = {};
        processedFilesCounts = {};
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
            "Backtracks per Decision",
            "ECDF",
        );
        drawTicksX(ctx, margin, W, H, MAX_BTD, 10);
        drawTicksY(ctx, margin, W, H, 1, 10);
    }

    /**
     * Draws chart with ECDFs of BT/D.
     */
    function drawChart() {
        const margin = { top: 40, right: 40, bottom: 60, left: 60 };
        const W = canvas.width;
        const H = canvas.height;

        ctx.clearRect(0, 0, W, H);

        // Iterate over all existing groups and create ECDF line for each one
        for (const groupId in histograms) {
            const histogram = histograms[groupId];
            const processedFiles = processedFilesCounts[groupId];
            drawEcdfLineFromHistogram(
                ctx,
                margin,
                W,
                H,
                histogram,
                NUM_BINS,
                MAX_BTD,
                processedFiles,
                getColorForGroup(groupId),
            );
        }

        drawAxes(margin, W, H);
        drawTitle(
            ctx,
            W,
            margin,
            "Empirical Cumulative Distribution Function of Backtracks per Decision",
        );
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
     * Adds BT/D per file to appropriate histograms
     */
    function onFileDone() {
        // Show only files where at least one decision was made
        if (decisionsInFile > 0) {
            const btd = backtracksInFile / decisionsInFile;
            const index = Math.min(
                Math.floor((btd / MAX_BTD) * NUM_BINS),
                NUM_BINS - 1,
            );

            for (const groupId of currentGroupIds) {
                // Data does not exist yet
                if (!histograms[groupId])
                    histograms[groupId] = new Array(NUM_BINS).fill(0);
                if (!processedFilesCounts[groupId])
                    processedFilesCounts[groupId] = 0;

                // Store in appropriate histogram
                histograms[groupId][index]++;
                processedFilesCounts[groupId]++;
            }
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
