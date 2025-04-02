import Papa from "papaparse";

let subscribers = []; // subscriber objects

/**
 * Subscribes to CSV parsing events.
 * 
 * @param {Function} onRow — Callback function called for each parsed row
 * @param {Function} onFileDone — Callback function called once per file
 * @param {Function} onAllDone — Callback function called once after all files
 * @param {Function|null} onFileStart - Callback function called once at the start of file's parsing
 * @returns {() => void} Function to unsubscribe
 */
export function subscribe(onRow, onFileDone, onAllDone, onFileStart) {
  const sub = { onRow, onFileDone, onAllDone, onFileStart };
  subscribers.push(sub);
  return () => {
    subscribers = subscribers.filter(s => s !== sub);
  };
}

/**
 * Reads and parses only the last line of a text file
 * 
 * @param {File} file - The log file to read from.
 * @param {number} [chunkSize=6144] - The maximum number of bytes to read from the end of the file.
 * @returns {Promise<string[]>} - A promise that resolves to an array of column values from the last line.
 *  
 */
export async function readParsedLastLine(file, chunkSize = 1024 * 6) {
  const offset = Math.max(0, file.size - chunkSize);
  const chunk = await file.slice(offset).text();
  const lines = chunk.trim().split("\n");
  const lastLine = lines[lines.length - 1];
  const parsed = Papa.parse(lastLine, { skipEmptyLines: true });
  return parsed.data[0];

}

/**
 * Parses multiple CSV files, invoking subscriber callbacks at each stage.
 * 
 * @param {Array<{ fileObject: File, groupIds: number[] }>} wrappedFiles - The log files to read from that imclude metadata.
 * @param {number} [chunkSize=6144] - The maximum number of bytes to read from the end of the file.
 *  
 */
export async function parseFiles(wrappedFiles, chunkSize = 1024 * 6) {
  // const timeStart = performance.now();
  const subs = subscribers.slice();

  for (const wrappedFile of wrappedFiles) {
    const realFile = wrappedFile.fileObject || wrappedFile; // Unwrap for parsing

    await Promise.all(
      subs.map(sub =>
        sub.onFileStart
          ? sub.onFileStart(realFile, wrappedFile.groupIds)
          : Promise.resolve()
      )
    );

    await new Promise((resolve, reject) => {
      Papa.parse(realFile, {
        skipEmptyLines: true,
        chunkSize,
        chunk({ data: rows }) {
          rows.forEach(row => subs.forEach(s => s.onRow?.(row)));
        },
        complete() {
          subs.forEach(s => s.onFileDone?.());
          resolve();
        },
        error: reject
      });
    });
  }

  // After all files are done
  subs.forEach(s => s.onAllDone?.());
  // const timeEnd = performance.now();
  // console.log(`Elapsed: ${(timeEnd - timeStart).toFixed(3)} ms`);
}

