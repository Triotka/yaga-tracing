
/**
 * Initializes a <canvas> and returns its 2D rendering context.
 *
 * @param {HTMLCanvasElement} canvas - The canvas element to initialize.
 * @param {number} [width=800] - Width of the canvas
 * @param {number} [height=400] - Height of the canvas
 * @param {string} [font="14px sans-serif"] - Font for the canvas
 * @returns {CanvasRenderingContext2D} - The 2D rendering context of the initialized canvas.
 */
export function initCanvas(canvas, width = 800, height = 400, font = "14px sans-serif") {
  const ctx = canvas.getContext("2d");
  canvas.width = width;
  canvas.height = height;
  ctx.font = font;
  return ctx;
}

/**
 * Draws a point.
 *
 * @param {CanvasRenderingContext2D} ctx - The canvas rendering context.
 * @param {number} x - The x-coordinate of the point.
 * @param {number} y - The y-coordinate of the point.
 * @param {number} [radius=4] - The radius of the point
 * @param {string} [color="black"] The color of the point.
 */
export function drawPoint(ctx, x, y, radius = 4, color = "black") {
  ctx.beginPath();
  ctx.arc(x, y, radius, 0, Math.PI * 2);
  ctx.fillStyle = color;
  ctx.fill();
}


/**
 * Draws a line between two points.
 *
 * @param {CanvasRenderingContext2D} ctx - The canvas rendering context.
 * @param {number} x1 - The starting point x-coordinate.
 * @param {number} y1 - The starting point y-coordinate.
 * @param {number} x2 - The ending point x-coordinate.
 * @param {number} y2 - The ending point y-coordinate.
 * @param {{color?: string, lineWidth?: number}} [opts] - Optional drawing options.
 * @param {string} [color="black"] - Stroke color of the line.
 * @param {number} [lineWidth=1] - Width of the line stroke.
 */
export function drawLine(ctx, x1, y1, x2, y2, color = "black", lineWidth = 1) {
  ctx.beginPath();
  ctx.moveTo(x1, y1);
  ctx.lineTo(x2, y2);
  ctx.strokeStyle = color;
  ctx.lineWidth = lineWidth;
  ctx.stroke();
}

/**
 * Draws the empty X and Y axes with their labels on the canvas.
 * 
 * @param {CanvasRenderingContext2D} ctx - The canvas rendering context.
 * @param {number} width - Width of the canvas.
 * @param {number} height - Height of the canvas.
 * @param {Object} margin - Margins object with {top, right, bottom, left} properties.
 * @param {string} labelX - Label text for the X axis.
 * @param {string} labelY - Label text for the Y axis.
 * @param {string} [font="14px sans-serif"] - Font style for axis labels.
 * @param {string} [color="black"] - Color of the axes.
 */
export function drawEmptyLabeledAxes(ctx, width, height, margin, labelX, labelY, font = "14px sans-serif", color = "black") {

  ctx.save();

  // Set style
  ctx.strokeStyle = color;
  ctx.fillStyle = color;
  ctx.font = font;

  drawLine(ctx, margin.left, height - margin.bottom, width - margin.right, height - margin.bottom); // Draw X line
  drawLine(ctx, margin.left, margin.top, margin.left, height - margin.bottom); // Draw Y axis  

  // Draw X axis label
  ctx.textAlign = "center";
  ctx.textBaseline = "top";
  ctx.fillText(labelX, width / 2, height - margin.bottom + 35);  // Draw the x-axis label centered

  // Draw Y axis label
  ctx.translate(margin.left - 45, height / 2);
  ctx.rotate(-Math.PI / 2);
  ctx.textAlign = "center";
  ctx.textBaseline = "top";
  ctx.fillText(labelY, 0, 0);

  ctx.restore();
}

/**
 * Draws a centered title text at the top of the canvas.
 *
 * @param {CanvasRenderingContext2D} ctx - The canvas context to draw on.
 * @param {number} width - The total width of the canvas.
 * @param {Object} margin - Margin object containing top, right, bottom, left values.
 * @param {string} title - The text to be displayed as the title.
 * @param {string} [color="black"] - Color of the title text.
 * @param {string} [font="16px sans-serif"] - Font style and size for the title.
 *
 */
export function drawTitle(ctx, width, margin, title, color = "black", font = "16px sans-serif") {
  ctx.font = font;
  ctx.textAlign = "center";
  ctx.fillStyle = color;
  ctx.fillText(title, width / 2, margin.top / 2);
}

/**
 * Displays a centered message on the canvas.
 *
 * @param {CanvasRenderingContext2D} ctx - The canvas context to draw on.
 * @param {string} message - The message to display.
 * @param {number} width - The width of the canvas.
 * @param {number} height - The height of the canvas.
 * @param {string} [color="black"] - The color of the message.
 * @param {string} [font="16px sans-serif"] - The font of the message.
 * 
 */
export function drawCenteredMessage(ctx, message, width, height, color = "black", font = "16px sans-serif") {
  ctx.fillStyle = color;
  ctx.font = font;
  ctx.textAlign = "center";
  ctx.textBaseline = "middle";
  ctx.fillText(message, width / 2, height / 2);
}


/**
 * Draws ECDF line based on histogram data.
 *
 * @param {CanvasRenderingContext2D} ctx - The canvas context to draw on.
 * @param {Object} margin - Margins of the chart {top, right, bottom, left}.
 * @param {number} width - The total width of the canvas.
 * @param {number} height - The total height of the canvas.
 * @param {number[]} histogram - The histogram to draw ECDF from.
 * @param {number} numBins - Number of bins in histogram.
 * @param {number} maxValue - The maximum data value represented by the last bin.
 * @param {number} numberOfEntries - Total count of all observations (i.e. sum of histogram).
 * @param {string} [color="blue"] - Color for the ECDF line.
 * @param {number} [opacity=0.7] - Opacity used for the chart data line
 */
export function drawEcdfLineFromHistogram(ctx, margin, width, height, histogram, numBins, maxValue, numberOfEntries, color = "blue", opacity = 0.7) {
  ctx.save();
  ctx.globalAlpha = opacity;

  let sumOfValues = 0;

  let previousPointX = 0;
  let previousPointY = 0;

  for (let i = 0; i < histogram.length; i++) {
    sumOfValues += histogram[i];

    // Compute ECDF point
    const x = (i / numBins) * maxValue;
    const y = sumOfValues / numberOfEntries;

    const pointX =
      margin.left + (x / maxValue) * (width - margin.left - margin.right);
    const pointY =
      height - margin.bottom - y * (height - margin.top - margin.bottom);

    if (i !== 0) {
      // Draw line only if you already have at least two points
      drawLine(ctx, previousPointX, previousPointY, pointX, pointY, color);
    }
    previousPointX = pointX;
    previousPointY = pointY;
  }

  if (histogram.length === 0) {
    drawCenteredMessage(ctx, "No data to display", width, height);
    return;
  }
  if (histogram.length === 1) {
    drawPoint(ctx, previousPointX, previousPointY);
  }
  ctx.restore();
}

/**
 * Draws a histogram.
 *
 * @param {CanvasRenderingContext2D} ctx - The canvas context to draw on.
 * @param {Object} margin - Margins of the chart {top, right, bottom, left}.
 * @param {number} width - The total width of the canvas.
 * @param {number} height - The total height of the canvas.
 * @param {number[]} histogram - The histogram data.
 * @param {number} maxCount - Maximum bin count, used to normalize bar heights.
 * @param {string} [color="blue"] - Color for the ECDF line.
 * @param {number} [opacity=0.7] - Opacity set for the histogram.
 *
 */
export function drawHistogram(ctx, margin, width, height, histogram, maxCount, color = "blue", opacity = 0.7) {
  if (histogram.length === 0) {
    drawCenteredMessage(ctx, "No data to display", width, height);
    return;
  }
  ctx.save();
  ctx.globalAlpha = opacity;
  const chartWidth = width - margin.left - margin.right;
  const chartHeight = height - margin.top - margin.bottom;

  const barWidth = chartWidth / histogram.length;

  histogram.forEach((count, i) => {
    const barHeight = (count / maxCount) * chartHeight;
    const x = margin.left + i * barWidth;
    const y = height - margin.bottom - barHeight;

    ctx.fillStyle = color;
    ctx.fillRect(x, y, barWidth - 1, barHeight); // -1 for space between bars
  });
  ctx.restore();
}

/**
 * @param {number} maxX
 * @param {number} ticksNumber 
 * @returns {number}
 */
function determineTickSize(maxX, ticksNumber) {
  const preciseNumber = maxX / ticksNumber;
  // round to nearest nice number like 10, 5, 2, 1, 0.5, etc.
  const scale = Math.floor(Math.log10(preciseNumber));
  const integerPart = preciseNumber * 10 ** -scale
  const roundedValue = integerPart <= 1.000000000001 ? 1 : integerPart <= 2.000000000001 ? 2 : integerPart <= 5.000000000001 ? 5 : 10;
  return roundedValue * 10 ** scale;
}

/**
 * Like `toPrecision`, but removes trailing zeros
 * @param {number} value
 * @param {number} precision
 * @returns {string}
 */
function formatNumberPrecision(value, precision) {
  return value.toPrecision(precision).replace(/([.]|(?<=[.].*[^0]))0*(?=e|$)/, '');
}

/**
 * Draws X-axis tick marks and value labels.
 *
 * @param {CanvasRenderingContext2D} ctx - The canvas context to draw on.
 * @param {Object} margin  – { top, right, bottom, left }
 * @param {number} width - The width of the canvas.
 * @param {number} height - The height of the canvas.
 * @param {number} maxX - Max value on X axis
 * @param {number} ticksNumber - Number of ticks on the axis  
 * @param {string} [color="black"] - The color of the message.
 * @param {string} [font="10px sans-serif"] - The font of the message.
 * 
 */
export function drawTicksX(ctx, margin, width, height, maxX, ticksNumber, font = "10px sans-serif", color = "black") {
  ctx.save();
  // Set style
  ctx.font = font;
  ctx.fillStyle = color;
  ctx.strokeStyle = color;

  ctx.textAlign = "center";
  ctx.textBaseline = "top";

  const tickSize = determineTickSize(maxX, ticksNumber);

  for (let i = 0; i <= ticksNumber; i += 1) {
	const tick = i * tickSize;
	if (tick > maxX)
	  break;
    const x = margin.left + (tick / maxX) * (width - margin.left - margin.right);
    drawLine(ctx, x, height - margin.bottom, x, height - margin.bottom + 5);
    ctx.fillText(formatNumberPrecision(tick, 3), x, height - margin.bottom + 8);
  }
  ctx.restore();
}
/**
 * Draws Y-axis tick marks and value labels.
 *
 * @param {CanvasRenderingContext2D} ctx - The canvas context to draw on.
 * @param {Object} margin  – { top, right, bottom, left }
 * @param {number} width - The width of the canvas.
 * @param {number} height - The height of the canvas.
 * @param {number} maxY - Max value on Y axis
 * @param {number} ticksNumber - Number of ticks on the axis  
 * @param {string} [color="black"] - The color of the message.
 * @param {string} [font="10px sans-serif"] - The font of the message.
 * 
 */
export function drawTicksY(ctx, margin, width, height, maxY, ticksNumber, font = "10px sans-serif", color = "black") {
  ctx.save();
  // Set style
  ctx.font = font;
  ctx.fillStyle = color;
  ctx.strokeStyle = color;

  ctx.textAlign = "right";
  ctx.textBaseline = "middle";

  const tickSize = determineTickSize(maxY, ticksNumber);

  for (let i = 0; i <= ticksNumber; i += 1) {
	const tick = i * tickSize;
	if (tick > maxY)
	  break;
    const y = height - margin.bottom - (tick / maxY) * (height - margin.top - margin.bottom);
    drawLine(ctx, margin.left, y, margin.left - 5, y);

    ctx.fillText(formatNumberPrecision(tick, 3), margin.left - 8, y);
  }
  ctx.restore();
}

/**
 * Plots an (x, y) scatter of data points.
 *
 * @param {CanvasRenderingContext2D} ctx - The canvas context to draw on.
 * @param {Array<Object>} dataPoints - Points to draw scatter plot from
 * @param {Object} margin  – { top, right, bottom, left }
 * @param {number} width - The width of the canvas.
 * @param {number} height - The height of the canvas.
 * @param {string} xKey - Object key name for the X value.
 * @param {string} yKey - Object key name for the Y value.
 * @param {number} xMax - Max value on X
 * @param {number} yMax - Max value on Y
 * @param {number} [radius=4] - Radius of the points  
 * @param {string} [color="black"] - The color of the message.
 * @param {number} [opacity=0.7] - The opacity set for the points
 */
export function drawScatterDataPoints(
  ctx,
  dataPoints,
  margin,
  width,
  height,
  xKey,
  yKey,
  xMax,
  yMax,
  radius = 4,
  color = "orange",
  opacity = 0.7
) {

  ctx.save();
  ctx.globalAlpha = opacity;

  dataPoints.sort((x, y) => x[xKey] - y[xKey]);

  dataPoints.forEach(point => {
    const xVal = point[xKey];
    const yVal = point[yKey];


    const xNorm = xMax === 0 ? 0 : xVal / xMax;
    const yNorm = yMax === 0 ? 0 : yVal / yMax;

    const pointX = margin.left + xNorm * (width - margin.left - margin.right);
    const pointY = height - margin.bottom - yNorm * (height - margin.top - margin.bottom);

    drawPoint(ctx, pointX, pointY, radius, color);
  });
  ctx.restore();
}

/**
 * Draws Line from medians.
 *
 * @param {CanvasRenderingContext2D} ctx - The canvas context to draw on.
 * @param {Object} margin  – { top, right, bottom, left }
 * @param {number} width - The width of the canvas.
 * @param {number} height - The height of the canvas.
 * @param {number} maxX - Max value on X
 * @param {number} maxY - Max value on Y
 * @param {number[]} sums - Sum values per bin
 * @param {number[]} counts - Count values per bin
 * @param {string} [lineColor="blue"] - The color of the line.
 * @param {number} [opacity=0.7] - Opacity for drawing
 * 
 */
export function drawChartLineFromMedians(ctx, margin, width, height, maxX, maxY, sums, counts, lineColor = "blue", opacity = 0.7) {

  ctx.save();
  ctx.globalAlpha = opacity;

  let previousPointX = 0;
  let previousPointY = 0;

  for (let i = 0; i < sums.length; i++) {
    const pointX =
      margin.left + (i / maxX) * (width - margin.left - margin.right);
    const median = sums[i] / counts[i];

    const normalizedY = maxY > 0 ? median / maxY : 0;
    const pointY =
      height -
      margin.bottom -
      normalizedY * (height - margin.top - margin.bottom);

    if (i !== 0) {
      drawLine(ctx, previousPointX, previousPointY, pointX, pointY, lineColor);
    }
   
    previousPointX = pointX;
    previousPointY = pointY;
  }

  if (sums.length === 1) {
    drawPoint(ctx, previousPointX, previousPointY, 2, lineColor);
    return;
  }
  ctx.restore();
  if (sums.length === 0) {
    drawCenteredMessage(ctx, "No data to display", width, height);
    return;
  }
}


/**
 * Generates a distinct HSL color for a given group ID.
 *
 * @param {string|number} groupId - Unique identifier for the group.
 * @returns {string} - HSL color string.
 */
export function getColorForGroup(groupId) {
  const hue = (parseInt(groupId) * 137) % 360; // Multiply by a prime to spread hues
  return `hsl(${hue}, 70%, 50%)`;
}

/**
   * Draws a connected line with optional points from binned jump ratio data.
   *
   * @param {CanvasRenderingContext2D} ctx - Canvas 2D context
   * @param {Object} margin - Margins {top, right, bottom, left}
   * @param {number} W - Full canvas width
   * @param {number} H - Full canvas height
   * @param {number[]} sumBins - Array of summed jump ratios per bin
   * @param {number[]} countBins - Array of counts per bin
   * @param {number} numBins - Total number of bins
   * @param {number} maxJumpRatioCap - Maximum jump ratio cap for scaling
   * @param {number} maxValue - Maximum value
   * @param {string} [lineColor="blue"] - Line color
   * @param {string} [pointsColor="red"] - Points color
   * @param {number} [opacity=0.7] - Opacity of the drawing
   */
export function drawLineFromBins(
  ctx,
  margin,
  W,
  H,
  sumBins,
  countBins,
  numBins,
  maxJumpRatioCap,
  maxValue,
  lineColor = "blue",
  pointsColor = "red",
  opacity = 0.7) {
  if (sumBins.length === 0) {
    drawCenteredMessage(ctx, "No data available", W, H);
    return;
  }

  ctx.save();
  ctx.globalAlpha = opacity;

  const plotWidth = W - margin.left - margin.right;
  const plotHeight = H - margin.top - margin.bottom;
  const binWidth = plotWidth / numBins;
  let previousX = null;
  let previousY = null;

  for (let i = 0; i < numBins; i++) {
    let cappedAverage = 0; // Stays 0 if countBins[i] <= 0
    if (countBins[i] > 0) {
      const avg = sumBins[i] / countBins[i];
      cappedAverage = Math.min(avg, maxJumpRatioCap);
    }

    const x = margin.left + binWidth * (i + 0.5);
    const ratio =
      cappedAverage / (maxValue || maxJumpRatioCap);
    const y = H - margin.bottom - ratio * plotHeight;
    if (previousX !== null && previousY !== null) {
      drawLine(ctx, previousX, previousY, x, y, lineColor);
    }
    drawPoint(ctx, x, y, 2, pointsColor);


    previousX = x;
    previousY = y;
  }
  ctx.restore();
}
