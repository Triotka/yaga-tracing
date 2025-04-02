<script>
    import { onMount, onDestroy } from "svelte";
    import { subscribe } from "../../lib/parser.js";
    import {
        drawTicksX,
        drawTicksY,
        drawEmptyLabeledAxes,
        initCanvas,
        drawTitle,
        drawHistogram,
        getColorForGroup,
    } from "../../lib/canvas.js";

    // CONSTANTS
    const NUM_BINS = 30; // Number of bins in histogram
    const MAX_BACKTRACKS_PER_WINDOW = 30; // A Number of backtracks per window that is bigger than this constant fall into a last bin
    const WINDOW_SIZE = 10;

    let canvas, ctx;
    let unsubscribe; // Stores the unsubscribe function

    let maxWindowOccurence = 0;

    // Count variables
    let decisionsInWindow = 0;
    let previousBacktracks = 0;
    let currentGroupIds = [];

    let histograms = {}; // Maps groupId to its histogram

    /**
     * Restarts data holders for the next chart data.
     *
     */
    function restartDataHolders() {
        maxWindowOccurence = 0;
        decisionsInWindow = 0;
        previousBacktracks = 0;
        currentGroupIds = [];
        histograms = {};
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
            "Backtracks per Window",
            "Number of Windows",
        );
        drawTicksX(ctx, margin, W, H, MAX_BACKTRACKS_PER_WINDOW, 10);
        drawTicksY(ctx, margin, W, H, maxWindowOccurence, 10);
    }

    /**
     * Calculates max window occurence and stores it in maxWindowOccurence variable.
     */
    function calculateMaxWindowOccurence() {
        for (const groupId in histograms) {
            for (let i = 0; i < histograms[groupId].length; i++) {
                maxWindowOccurence = Math.max(
                    maxWindowOccurence,
                    histograms[groupId][i],
                );
            }
        }
    }

    /**
     * Draws backtrack density histogram
     */
    function drawChart() {
        const margin = { top: 40, right: 40, bottom: 60, left: 60 };
        const W = canvas.width;
        const H = canvas.height;
        ctx.clearRect(0, 0, W, H);

        for (const groupId in histograms) {
            const histogram = histograms[groupId];
            drawHistogram(
                ctx,
                margin,
                W,
                H,
                histogram,
                maxWindowOccurence,
                getColorForGroup(groupId),
            );
        }
        drawAxes(margin, W, H);
        drawTitle(ctx, W, margin, "Backtrack Density Histogram");
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
     * Computes number of backtracks in a fixed window in the file and stores it in appropriate group histogram.
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

                // Store in histogram
                const index = Math.min(
                    Math.floor(
                        (backtracksInWindow / MAX_BACKTRACKS_PER_WINDOW) *
                            NUM_BINS,
                    ),
                    NUM_BINS - 1,
                );

                for (const groupId of currentGroupIds) {
                    // Data does not exist yet
                    if (!histograms[groupId])
                        histograms[groupId] = new Array(NUM_BINS).fill(0);

                    // Store in appropriate histogram
                    histograms[groupId][index]++;
                }

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
        // Reset for the next file and getting rid of unfinished partial window
        decisionsInWindow = 0;
        previousBacktracks = 0; // Backtracks cannot be made without decisions
    }

    /**
     * Callback function called after all CSV fileshave been parsed.
     * Calculate Y axis maximum, draws a chart and restarts data holders.
     */
    function onAllDone() {
        calculateMaxWindowOccurence();
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
