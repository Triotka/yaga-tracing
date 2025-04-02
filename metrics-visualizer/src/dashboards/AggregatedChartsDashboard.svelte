<script>
  import { tick } from "svelte";
  import { parseFiles } from "../lib/parser.js";
  import html2canvas from "html2canvas";
  import JSZip from "jszip";
  import { saveAs } from "file-saver";

  // Trashing analysis
  import EcdfBtdChart from "../charts/trashing/EcdfBtdChart.svelte";
  import DecisionWindowsVsBtdChart from "../charts/trashing/DecisionWindowsVsBtdChart.svelte";
  import ScatterDecisionsVsBtdChart from "../charts/trashing/ScatterDecisionsVsBtdChart.svelte";

  // Backtrack clustering
  import EcdfBacktrackSpacingChart from "../charts/backtrack_clustering/EcdfBacktrackSpacingChart.svelte";
  import BacktrackDensityHistogramChart from "../charts/backtrack_clustering/BacktrackDensityHistogramChart.svelte";
  import ScatterDecisionVsAvgClusterChart from "../charts/backtrack_clustering/ScatterDecisionsVsAvgClusterChart.svelte";

  // Shallow backtracks
  import EcdfBacktrackJumpRatioChart from "../charts/backtrack_shallow/EcdfBacktrackJumpRatioChart.svelte";
  import ScatterDecisionsVsAvgJumpRatioChart from "../charts/backtrack_shallow/ScatterDecisionVsAvgJumpRatioChart.svelte";
  import ProgressVsAvgJumpRatioChart from "../charts/backtrack_shallow/ProgressVsAvgJumpRatioChart.svelte";

  export let files = [];

  // Define your chart groups and components
  const groups = [
    {
      name: "Trashing Analysis",
      charts: [
        {
          id: "ecdfBtd",
          label: "ECDF of Backtracks per Decision",
          component: EcdfBtdChart,
        },
        {
          id: "decisionWindowsVsBtd",
          label: "Decision Window Index vs Backtracks per Decision",
          component: DecisionWindowsVsBtdChart,
        },
        {
          id: "scatterDecisionsVsBtd",
          label: "Total Decisions vs. Overall Backtracks per Decision",
          component: ScatterDecisionsVsBtdChart,
        },
      ],
    },
    {
      name: "Backtrack Burst Clusters",
      charts: [
        {
          id: "ecdfBacktrackSpacing",
          label: "ECDF of Backtrack Spacing",
          component: EcdfBacktrackSpacingChart,
        },
        {
          id: "backtrackDensityHistogram",
          label: "Backtrack Density Histogram",
          component: BacktrackDensityHistogramChart,
        },
        {
          id: "scatterDecisionsVsAvgCluster",
          label: "Total Decisions vs. Average Cluster Size",
          component: ScatterDecisionVsAvgClusterChart,
        },
      ],
    },
    {
      name: "Shallow Backtracks",
      charts: [
        {
          id: "ecdfBacktrackJumpRatio",
          label: "ECDF of Backtrack Jump Ratio",
          component: EcdfBacktrackJumpRatioChart,
        },
        {
          id: "progressVsAvgJumpRatio",
          label: "Progress vs. Average Jump Ratio",
          component: ProgressVsAvgJumpRatioChart,
        },
        {
          id: "scatterDecisionsVsAvgJumpRatio",
          label: "Total Decisions vs. Average Jump Ratio",
          component: ScatterDecisionsVsAvgJumpRatioChart,
        },
      ],
    },
  ];

  let selected = {};
  groups.forEach((g) => g.charts.forEach((c) => (selected[c.id] = false)));

  /**
   * Function to generate charts used when clicked on Generate button.
   */
  async function generate() {
    await tick();
    parseFiles(files);
  }

  /**
   * Function to download mounted charts to zip.
   */
  async function downloadChartsAsZip() {
    await tick();

    const chartsElements = Array.from(
      document.querySelectorAll(".export-wrapper"),
    );
    if (!chartsElements.length) {
      alert("No charts available!");
      return;
    }
    const zip = new JSZip();

    await Promise.all(
      chartsElements.map(async (exportChart) => {
        const id = exportChart.dataset.id; // Use data-id added in export-wrapper
        const canvas = await html2canvas(exportChart, { scale: 2 });
        return new Promise((resolve) => {
          canvas.toBlob((blob) => {
            zip.file(`${id}.png`, blob);
            resolve();
          }, "image/png");
        });
      }),
    );

    zip.generateAsync({ type: "blob" }).then((content) => {
      saveAs(content, "charts.zip");
    });
  }
</script>

<h2>Aggregate Dashboard</h2>

{#each groups as group}
  <section class="charts-group">
    <h3>{group.name}</h3>
    <div class="checkbox-row">
      {#each group.charts as chart}
        <label>
          <input type="checkbox" bind:checked={selected[chart.id]} />
          {chart.label}
        </label>
      {/each}
    </div>
  </section>
{/each}

<button class="action-btn" on:click={generate}> Generate </button>
<button class="action-btn" on:click={downloadChartsAsZip}>
  Download Chart Images
</button>

{#each groups as group}
  <section class="charts-group">
    {#each group.charts as chart (chart.id)}
      {#if selected[chart.id]}
        <div class="export-wrapper" data-id={chart.id}>
          <svelte:component this={chart.component} />
        </div>
      {/if}
    {/each}
  </section>
{/each}

<style>
  .charts-group {
    margin-bottom: 2rem;
    border-top: 1px solid #ddd;
    padding-top: 1rem;
  }
  .charts-group h3 {
    margin-bottom: 0.5rem;
  }
  .checkbox-row {
    display: flex;
    flex-wrap: wrap;
    gap: 1rem;
    margin-bottom: 1rem;
  }
  .action-btn {
    padding: 0.5rem 1rem;
    font-size: 1rem;
    margin-bottom: 2rem;
  }
</style>
