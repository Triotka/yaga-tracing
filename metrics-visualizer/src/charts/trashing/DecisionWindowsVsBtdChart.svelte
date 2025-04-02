<script>
    import { onMount, onDestroy } from "svelte";
    import { subscribe } from "../../lib/parser.js";
    import {
        initCanvas,
        drawTitle,
        drawEmptyLabeledAxes,
        drawChartLineFromMedians,
        drawTicksX,
        drawTicksY,
        getColorForGroup,
    } from "../../lib/canvas.js";

    // CONSTANTS
    const WINDOW_SIZE = 500; // Number of decisions per window, greater than 0
    const MAX_WINDOW_NUMBER = 1_000_000; // Maximum number of windows

    let canvas, ctx;
    let unsubscribe; // Stores the unsubscribe function

    let medianMax = 0; // Max BT/D median of all files used for drawing the Y axis
    let windowIndexMax = 0; // Maximum window index of all files used for drawing the X axis

    // Counting variables
    let windowIndex = 0;
    let decisionsInWindow = 0;
    let previousBacktracks = 0;
    let currentGroupIds = [];

    let btdSumsByGroup = {}; // Maps groupId to its BT/D sums
    let btdCountsByGroup = {}; // Maps groupId to its BT/D count

    /**
     * Restarts data holders for the next chart data.
     *
     */
    function restartDataHolders() {
        medianMax = 0;
        windowIndexMax = 0;
        windowIndex = 0;
        decisionsInWindow = 0;
        previousBacktracks = 0;
        currentGroupIds = [];
        btdSumsByGroup = {};
        btdCountsByGroup = {};
    }

    /**
     * Draws Line chart of window indexes vs. BT/D medians.
     */
    function drawChart() {
        const margin = { top: 40, right: 40, bottom: 60, left: 60 };
        const W = canvas.width;
        const H = canvas.height;
        ctx.clearRect(0, 0, W, H);

        // Iterate over all existing groups and create line for each one
        for (const groupId in btdSumsByGroup) {
            const btdSums = btdSumsByGroup[groupId];
            const btdCounts = btdCountsByGroup[groupId];
            drawChartLineFromMedians(
                ctx,
                margin,
                W,
                H,
                windowIndexMax,
                medianMax,
                btdSums,
                btdCounts,
                getColorForGroup(groupId),
            );
        }
        drawAxes(margin, W, H);
        drawTitle(
            ctx,
            W,
            margin,
            "Windows Decisions Indexes vs Backtracks per Decision",
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
            "Decision Window Index",
            "Backtracks per Decision Mean",
        );
        drawTicksX(ctx, margin, W, H, windowIndexMax, 10);
        drawTicksY(ctx, margin, W, H, medianMax, 10);
    }
    /**
     * Calculates data's max values and stores them in global values.
     */
    function calculateMaxima() {
        for (const groupId in btdSumsByGroup) {
            for (let i = 0; i < btdSumsByGroup[groupId].length; i++) {
                windowIndexMax = Math.max(
                    btdSumsByGroup[groupId].length - 1,
                    windowIndexMax,
                );
                const median =
                    btdSumsByGroup[groupId][i] / btdCountsByGroup[groupId][i];
                medianMax = Math.max(medianMax, median);
            }
        }
    }
    /**
     * Stores BT/D in group sum and increases count.
     */
    function storeNewBTD(btd, windowIndex, groupId) {
        if (windowIndex > MAX_WINDOW_NUMBER) {
            console.warn(
                "Exceeded max window limit some windows were thrown away, adjust constant",
            );
            return;
        }
        if (!btdSumsByGroup[groupId]) btdSumsByGroup[groupId] = [];
        if (!btdCountsByGroup[groupId]) btdCountsByGroup[groupId] = [];

        if (windowIndex >= btdSumsByGroup[groupId].length) {
            btdSumsByGroup[groupId].push(0);
            btdCountsByGroup[groupId].push(0);
        }

        btdSumsByGroup[groupId][windowIndex] += btd;
        btdCountsByGroup[groupId][windowIndex]++;
    }

    /**
     * Callback function to parse a single CSV file.
     * Stores current file's group IDs in the global variable.
     */
    async function onFileStart(file, groupIds) {
        currentGroupIds = groupIds;
        // Reset for the next file and getting rid of unfinished partial window
        decisionsInWindow = 0;
        previousBacktracks = 0; // Backtracks cannot be made without decisions
        windowIndex = 0;
    }

    /**
     * Callback function to process a single CSV row.
     * Computes BT/D for fixed window in the file and stores it in histogram
     *
     * @param {Array} row - A single row from the CSV.
     */
    function onRow(row) {
        const origin = row[0];
        const event = row[1];

        if (origin === "core" && event === "decision") {
            decisionsInWindow++;
            if (decisionsInWindow === WINDOW_SIZE) {
                const currentBacktracks = +row[6];

                const backtracksInWindow =
                    currentBacktracks - previousBacktracks;
                const btd = backtracksInWindow / decisionsInWindow;

                for (const groupId of currentGroupIds) {
                    storeNewBTD(btd, windowIndex, groupId);
                }

                windowIndex++;

                decisionsInWindow = 0;
                previousBacktracks = currentBacktracks;
            }
        }
    }

    /**
     * Callback function to parse a single CSV file.
     * Restarts count values.
     */
    function onFileDone() {
    }

    /**
     * Callback function called after all CSV fileshave been parsed.
     * Calculates maxima, draws a chart and restarts data holders.
     */
    function onAllDone() {
        calculateMaxima();
        drawChart();
        restartDataHolders();
    }

    onDestroy(() => {
        unsubscribe?.(); // Clean up
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
