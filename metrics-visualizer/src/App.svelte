<script>
  import AggregatedChartsDashboard from "./dashboards/AggregatedChartsDashboard.svelte";
  import { getColorForGroup } from "./lib/canvas";

  let groupCounter = 0; // A counter to generate unique group IDs
  let uploadedFiles = {}; // Map of files by unique names to file and its groupIds

  $: allFiles = Object.values(uploadedFiles).map((file) => file);
  $: groupIds = Array.from(
    new Set(Object.values(uploadedFiles).flatMap((file) => file.groupIds)),
  ).sort((a, b) => a - b);

  /**
   * Generates file's key from its metadata.
   *
   * @param {File} file - The file to generate key for.
   */
  function generateFileKey(file) {
    return `${file.name}-${file.size}-${file.lastModified}`;
  }

  /**
   * Handles file upload input event. Each upload has unique group ID which is assigned to the uploaded files.
   *
   * @param {Event} e - The change event from the file input element.
   */
  function handleFileUpload(e) {
    const files = Array.from(e.target.files);
    if (files.length === 0) return;

    groupCounter += 1;
    const groupId = groupCounter;

    files.forEach((file) => {
      const key = generateFileKey(file); // Use file metadata as key to check if file was uploaded before
      if (uploadedFiles[key]) {
        if (!uploadedFiles[key].groupIds.includes(groupId)) {
          uploadedFiles[key].groupIds = [
            ...uploadedFiles[key].groupIds,
            groupId,
          ];
        }
      } else {
        // Not uploaded before add as new entry
        uploadedFiles = {
          ...uploadedFiles,
          [key]: {
            fileObject: file,
            groupIds: [groupId],
          },
        };
      }
    });

    // Reset input so same file can be uploaded again if desired
    e.target.value = null;
  }
</script>

<h1>SMT Metrics Dashboard</h1>
<input type="file" accept=".csv" multiple on:change={handleFileUpload} />

{#if groupIds.length > 0}
  <div class="groups">
    {#each groupIds as gid}
      <div class="group">
        <h2>
          <span
            class="group-color"
            style="background-color: {getColorForGroup(gid)}"
          ></span>
          Group {gid}
        </h2>
        <ul>
          {#each allFiles.filter((f) => f.groupIds.includes(gid)) as file}
            <li>{file.fileObject.name}</li>
          {/each}
        </ul>
      </div>
    {/each}
  </div>
{/if}

{#if allFiles.length > 0}
  <AggregatedChartsDashboard files={allFiles} />
{:else}
  <p>Please upload one or more CSV files to begin.</p>
{/if}

<style>
  .group-color {
    display: inline-block;
    width: 16px;
    height: 16px;
    margin-right: 0.5rem;
    vertical-align: middle;
    border-radius: 3px;
  }

  ul {
    list-style-type: none;
    padding-left: 0;
    margin: 0;
  }
</style>
