/**
 * app.js — complete frontend logic
 *
 * Sections:
 *   1. Constants & state
 *   2. D3 setup
 *   3. Graph render
 *   4. Sidebar renderers
 *   5. Actions (step, back, play, reset, fit)
 *   6. History management
 *   7. Event listeners
 */

// ─────────────────────────────────────────────────────────────────────────────
// 1. Constants & state
// ─────────────────────────────────────────────────────────────────────────────

const SERVER = "http://localhost:5000";

// Colors must be visually distinct on a dark background.
// Index corresponds to the integer in the colors[] array from C++.
// Index 0 = uncoloured/error (colour() returns 0 on failure) — shown as red
const PALETTE = [
    "#07090f",  // 0 = uncoloured (matches background)
  "#4a90d9","#3db87a","#d9943a","#c45fbd",
  "#7a6fd4","#3ab8c4","#d97a3a","#8bc43a","#d95a5a",
];
const nodeColor = (i) => PALETTE[i % PALETTE.length];

// History: array of GraphState objects, one per completed step.
// historyIdx points to what's currently rendered.
// Going "back" just decrements historyIdx and re-renders — no server call.
const history = [];
let historyIdx = -1;

let currentState = { n: 0, matrix: [], colors: [], iteration: -1, status: "idle" };
let selectedNodeId = null;
let isPlaying = false;
let playTimer = null;
let posCache = {};  // {nodeId: {x,y}} — persists positions across re-renders

// ─────────────────────────────────────────────────────────────────────────────
// 2. D3 setup — mount SVG, configure zoom, create layer groups
// ─────────────────────────────────────────────────────────────────────────────

const panel = document.getElementById("graph-panel");

const svg = d3.select(panel).append("svg")
  .attr("width", "100%").attr("height", "100%");

// Subtle grid texture
const defs = svg.append("defs");
defs.append("pattern")
  .attr("id","grid").attr("width",40).attr("height",40)
  .attr("patternUnits","userSpaceOnUse")
  .append("path").attr("d","M40 0L0 0 0 40")
  .attr("fill","none").attr("stroke","#0c1018").attr("stroke-width","1");
svg.append("rect").attr("width","100%").attr("height","100%").attr("fill","url(#grid)");

const zoom = d3.zoom().scaleExtent([0.08, 10])
  .on("zoom", (e) => root.attr("transform", e.transform));
svg.call(zoom);
svg.on("click", () => selectNode(null));  // deselect on canvas click

const root  = svg.append("g");
const linkG = root.append("g").attr("class", "link-layer");
const nodeG = root.append("g").attr("class", "node-layer");

let simulation = null;

// ─────────────────────────────────────────────────────────────────────────────
// 3. Graph render — derives nodes/links from state, runs D3 force simulation
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Derive D3-friendly arrays from the raw adjacency matrix.
 * Only nodes with degree > 0 are included (as required).
 */
function deriveGraph({ n, matrix, colors }) {
  const nodes = [], links = [], seen = new Set();
  for (let i = 0; i < n; i++) {
    if (matrix[i]?.[n] > 0)
      nodes.push({ id: i, color: colors[i], degree: matrix[i][n] });
  }
  const active = new Set(nodes.map(d => d.id));
  for (let i = 0; i < n; i++) {
    if (!active.has(i)) continue;
    for (let j = i + 1; j < n; j++) {
      if (!active.has(j) || !matrix[i][j]) continue;
      const key = `${i}-${j}`;
      if (!seen.has(key)) { seen.add(key); links.push({ source: i, target: j }); }
    }
  }
  return { nodes, links };
}

function renderGraph(state) {
  const { nodes, links } = deriveGraph(state);
  const W = panel.clientWidth, H = panel.clientHeight;

  document.getElementById("empty").style.display = nodes.length ? "none" : "flex";

  // Persist positions so nodes don't scatter on every re-render
  nodeG.selectAll(".node-g").each(d => { posCache[d.id] = { x: d.x, y: d.y }; });
  nodes.forEach(n => {
    const c = posCache[n.id];
    n.x = c ? c.x : W / 2 + (Math.random() - .5) * 220;
    n.y = c ? c.y : H / 2 + (Math.random() - .5) * 220;
  });

  if (simulation) simulation.stop();

  simulation = d3.forceSimulation(nodes)
    .force("link",    d3.forceLink(links).id(d => d.id).distance(88).strength(.5))
    .force("charge",  d3.forceManyBody().strength(-240))
    .force("center",  d3.forceCenter(W / 2, H / 2 - 14))  // -14 for scrubber bar
    .force("collide", d3.forceCollide(26))
    .alphaDecay(.038);

  // ── Links ──────────────────────────────────────────────────────
  const lSel = linkG.selectAll(".link")
    .data(links, d => `${d.source.id ?? d.source}-${d.target.id ?? d.target}`);

  lSel.enter().append("line").attr("class", "link")
    .merge(lSel)
    .classed("hi", d =>
      selectedNodeId !== null &&
      (d.source.id === selectedNodeId || d.target.id === selectedNodeId));
  lSel.exit().remove();

  // ── Nodes ──────────────────────────────────────────────────────
  const nSel = nodeG.selectAll(".node-g").data(nodes, d => d.id);

  const entered = nSel.enter().append("g").attr("class", "node-g")
    .call(d3.drag()
      .on("start", (e, d) => { if (!e.active) simulation.alphaTarget(.3).restart(); d.fx = d.x; d.fy = d.y; })
      .on("drag",  (e, d) => { d.fx = e.x; d.fy = e.y; })
      .on("end",   (e, d) => { if (!e.active) simulation.alphaTarget(0); d.fx = null; d.fy = null; }))
    .on("click",   (e, d) => { e.stopPropagation(); selectNode(d); });

  entered.append("circle").attr("class", "node-ring");
  entered.append("circle").attr("class", "node-circle");
  entered.append("text").attr("class", "node-label");
  nSel.exit().remove();

  const all = entered.merge(nSel);
  const r = d => 11 + Math.min(d.degree * 1.4, 8);   // radius scales with degree

  all.select(".node-ring")
    .attr("r", d => r(d) + 7)
    .attr("stroke", d => nodeColor(d.color)).attr("stroke-width", 1.5);

  all.select(".node-circle")
    .attr("r", r)
    .attr("fill", d => nodeColor(d.color))
    .attr("stroke", d => nodeColor(d.color))
    .attr("stroke-width", d => d.id === selectedNodeId ? 3 : 1.5)
    .style("filter", d => `drop-shadow(0 0 6px ${nodeColor(d.color)}77)`);

  all.select(".node-label").text(d => d.id);

  simulation.on("tick", () => {
    linkG.selectAll(".link")
      .attr("x1", d => d.source.x).attr("y1", d => d.source.y)
      .attr("x2", d => d.target.x).attr("y2", d => d.target.y);
    nodeG.selectAll(".node-g")
      .attr("transform", d => `translate(${d.x},${d.y})`);
  });

  return { nodes, links };
}

// ─────────────────────────────────────────────────────────────────────────────
// 4. Sidebar renderers
// ─────────────────────────────────────────────────────────────────────────────

/** Compute graph-theoretic properties for the properties panel. */
function computeProps(state, nodes, links) {
  const n = nodes.length;
  const e = links.length;
  const maxEdges = n * (n - 1) / 2;
  const density  = maxEdges > 0 ? e / maxEdges : 0;
  const avgDeg   = n > 0 ? nodes.reduce((s, d) => s + d.degree, 0) / n : 0;
  const colors   = new Set(nodes.map(d => d.color)).size;
  const connected = isConnected(state, nodes);
  return { density, avgDeg, colors, connected };
}

/** BFS to check if all active nodes are reachable from any one node. */
function isConnected(state, nodes) {
  if (nodes.length <= 1) return true;
  const active  = new Set(nodes.map(d => d.id));
  const visited = new Set();
  const queue   = [nodes[0].id];
  visited.add(nodes[0].id);
  while (queue.length) {
    const cur = queue.shift();
    for (let j = 0; j < state.n; j++) {
      if (active.has(j) && !visited.has(j) && state.matrix[cur][j]) {
        visited.add(j); queue.push(j);
      }
    }
  }
  return visited.size === nodes.length;
}

function renderProps(state, nodes, links) {
  if (!nodes.length) {
    ["pDensity","pAvgDeg","pConnected","pColors"].forEach(id =>
      document.getElementById(id).textContent = "—");
    return;
  }
  const { density, avgDeg, colors, connected } = computeProps(state, nodes, links);

  const setVal = (id, text, cls) => {
    const el = document.getElementById(id);
    el.textContent = text;
    el.className = "prop-card-v" + (cls ? " " + cls : "");
  };

  setVal("pDensity",   density.toFixed(3));
  setVal("pAvgDeg",    avgDeg.toFixed(2));
  setVal("pConnected", connected ? "Yes" : "No", connected ? "good" : "bad");
  setVal("pColors",    colors);
}

function renderColorMap(nodes) {
  const el = document.getElementById("colorMap");
  if (!nodes.length) { el.innerHTML = `<span class="no-data">No data</span>`; return; }
  const counts = {};
  nodes.forEach(n => { counts[n.color] = (counts[n.color] || 0) + 1; });
  el.innerHTML = Object.entries(counts).sort((a,b) => +a[0] - +b[0]).map(([ci, cnt]) =>
    `<div class="leg-row">
       <span class="leg-dot" style="background:${nodeColor(+ci)}"></span>
       <span>Color ${ci}</span>
       <span class="leg-cnt">${cnt} node${cnt > 1 ? "s" : ""}</span>
     </div>`).join("");
}

function renderMatrix(state, nodes) {
  const { n, matrix } = state;
  const ids = nodes.map(d => d.id);
  const tbl = document.getElementById("matrix");

  if (!ids.length) {
    tbl.innerHTML = `<tr><td class="no-data" style="padding:8px">No active nodes</td></tr>`;
    return;
  }

  let h = `<thead><tr><th></th>${ids.map(i => `<th>${i}</th>`).join("")}<th class="degh">d</th></tr></thead><tbody>`;
  ids.forEach(i => {
    h += `<tr><th>${i}</th>`;
    ids.forEach(j => {
      const v   = matrix[i][j];
      const cls = [
        i === j                   ? "self" : "",
        v && i !== j              ? "e1"   : "",
        i === selectedNodeId      ? "hl"   : "",
      ].filter(Boolean).join(" ");
      h += `<td class="${cls}">${i === j ? "·" : v}</td>`;
    });
    h += `<td class="deg">${matrix[i][n]}</td></tr>`;
  });
  tbl.innerHTML = h + "</tbody>";
}

function renderInspector(node, state) {
  const empty  = document.getElementById("inspEmpty");
  const detail = document.getElementById("inspDetail");

  if (!node) {
    empty.style.display = "block";
    detail.style.display = "none";
    return;
  }
  empty.style.display = "none";
  detail.style.display = "block";

  const { n, matrix } = state;
  const nbrs = [];
  for (let j = 0; j < n; j++) if (j !== node.id && matrix[node.id]?.[j]) nbrs.push(j);

  detail.innerHTML = `
    <div class="irow"><span class="il">Node</span>    <span class="iv">${node.id}</span></div>
    <div class="irow"><span class="il">Degree</span>  <span class="iv">${node.degree}</span></div>
    <div class="irow"><span class="il">Color</span>   <span class="iv"><span class="swatch" style="background:${nodeColor(node.color)}"></span>${node.color}</span></div>
    <div class="irow"><span class="il">Neighbors</span><span class="iv">${nbrs.length}</span></div>
    <div class="chips">${nbrs.map(j => `<span class="chip">${j}</span>`).join("")}</div>`;
}

/** Update header stats, all sidebar panels, and graph edge highlights. */
function applyState(state, skipGraphRender = false) {
  currentState = state;

  // Header
  document.getElementById("hIter").textContent  = state.iteration >= 0 ? state.iteration : "—";
  document.getElementById("hNodes").textContent = state.n || "—";

  const { nodes, links } = skipGraphRender
    ? deriveGraph(state)
    : renderGraph(state);

  document.getElementById("hEdges").textContent = links.length || "—";

  // Status badge
  const badge = document.getElementById("badge");
  const text  = document.getElementById("badgeText");
  badge.className = `badge ${state.status}`;
  text.textContent = state.status === "done"    ? "Done"
                   : state.status === "running" ? (state.message || `Iter ${state.iteration}`)
                   : "Idle";

  // Sidebar
  renderProps(state, nodes, links);
  renderColorMap(nodes);
  renderMatrix(state, nodes);
  renderInspector(
    selectedNodeId !== null ? nodes.find(n => n.id === selectedNodeId) || null : null,
    state
  );

  // Step button state
  document.getElementById("stepBtn").disabled = state.status === "done";
  document.getElementById("playBtn").disabled = state.status === "done";
}

// ─────────────────────────────────────────────────────────────────────────────
// 5. Node selection
// ─────────────────────────────────────────────────────────────────────────────

function selectNode(node) {
  selectedNodeId = node ? node.id : null;

  // Highlight edges connected to the selected node
  linkG.selectAll(".link").classed("hi", d =>
    node && (d.source.id === node.id || d.target.id === node.id));

  // Thicken selected node border
  nodeG.selectAll(".node-circle")
    .attr("stroke-width", d => d.id === selectedNodeId ? 3 : 1.5);

  renderInspector(node, currentState);

  // Re-render matrix to highlight the selected row
  renderMatrix(currentState, deriveGraph(currentState).nodes);
}

// ─────────────────────────────────────────────────────────────────────────────
// 6. History — store every state so you can scrub back without re-running C++
// ─────────────────────────────────────────────────────────────────────────────

function pushHistory(state) {
  // If user went back then stepped forward, truncate the future
  history.splice(historyIdx + 1);
  history.push(structuredClone(state));
  historyIdx = history.length - 1;
  updateScrubber();
}

function updateScrubber() {
  const bar    = document.getElementById("scrubber-bar");
  const slider = document.getElementById("historySlider");
  const label  = document.getElementById("historyLabel");
  const back   = document.getElementById("backBtn");

  if (history.length <= 1) {
    bar.style.display = "none";
  } else {
    bar.style.display = "flex";
    slider.max   = history.length - 1;
    slider.value = historyIdx;
    label.textContent = `${historyIdx + 1} / ${history.length}`;
  }
  back.disabled = historyIdx <= 0;
}

function goToHistory(idx) {
  if (idx < 0 || idx >= history.length) return;
  historyIdx = idx;
  // Re-render from cached state — no server round trip
  applyState(history[idx]);
  updateScrubber();
}

// ─────────────────────────────────────────────────────────────────────────────
// 7. Actions
// ─────────────────────────────────────────────────────────────────────────────

async function doStep() {
  if (currentState.status === "done") return;
  setStepBusy(true);
  try {
    const state = await fetch(`${SERVER}/step`, { method: "POST" }).then(r => r.json());
    pushHistory(state);
    applyState(state);
  } catch (e) {
    setStatus("error", `Error: ${e.message}`);
  } finally {
    setStepBusy(false);
  }
}

function doBack() {
  if (historyIdx <= 0) return;
  goToHistory(historyIdx - 1);
}

function togglePlay() {
  isPlaying = !isPlaying;
  document.getElementById("playBtn").textContent = isPlaying ? "⏸ PAUSE" : "▶ PLAY";

  if (isPlaying) {
    const tick = async () => {
      if (!isPlaying || currentState.status === "done") { stopPlay(); return; }
      await doStep();
      const msPerStep = 1100 - document.getElementById("speedSlider").value * 100;
      playTimer = setTimeout(tick, msPerStep);
    };
    tick();
  } else {
    stopPlay();
  }
}

function stopPlay() {
  isPlaying = false;
  clearTimeout(playTimer);
  document.getElementById("playBtn").textContent = "▶ PLAY";
}

async function doReset() {
  stopPlay();
  selectedNodeId = null;
  posCache = {};
  history.length = 0;
  historyIdx = -1;

  nodeG.selectAll("*").remove();
  linkG.selectAll("*").remove();
  document.getElementById("empty").style.display = "flex";
  document.getElementById("scrubber-bar").style.display = "none";
  document.getElementById("backBtn").disabled = true;

  const state = await fetch(`${SERVER}/reset`, { method: "POST" }).then(r => r.json());
  applyState(state);
}

function doFit() {
  const bounds = root.node().getBBox();
  if (!bounds.width || !bounds.height) return;
  const W = panel.clientWidth, H = panel.clientHeight;
  const scale = Math.min(.88 * W / bounds.width, .88 * H / bounds.height, 3);
  svg.transition().duration(420).call(
    zoom.transform,
    d3.zoomIdentity
      .translate(W / 2 - scale * (bounds.x + bounds.width / 2),
                 H / 2 - scale * (bounds.y + bounds.height / 2))
      .scale(scale)
  );
}

function doExport() {
  const blob = new Blob([JSON.stringify(currentState, null, 2)], { type: "application/json" });
  const a    = Object.assign(document.createElement("a"), {
    href: URL.createObjectURL(blob),
    download: `graph_iter_${currentState.iteration}.json`,
  });
  a.click();
}

function setStepBusy(busy) {
  document.getElementById("stepBtn").disabled = busy || currentState.status === "done";
}

function setStatus(cls, msg) {
  document.getElementById("badge").className    = `badge ${cls}`;
  document.getElementById("badgeText").textContent = msg;
}

// ─────────────────────────────────────────────────────────────────────────────
// 8. Event listeners
// ─────────────────────────────────────────────────────────────────────────────

document.getElementById("stepBtn").addEventListener("click", doStep);
document.getElementById("backBtn").addEventListener("click", doBack);
document.getElementById("playBtn").addEventListener("click", togglePlay);
document.getElementById("resetBtn").addEventListener("click", doReset);
document.getElementById("fitBtn").addEventListener("click", doFit);
document.getElementById("exportBtn").addEventListener("click", doExport);

document.getElementById("historySlider").addEventListener("input", e => {
  goToHistory(+e.target.value);
});

document.addEventListener("keydown", e => {
  const tag = document.activeElement.tagName;
  if (tag === "INPUT" || tag === "TEXTAREA") return;
  if (e.key === "Enter")      doStep();
  if (e.key === "ArrowLeft")  doBack();
  if (e.key === "f" || e.key === "F") doFit();
  if (e.key === " ")          { e.preventDefault(); togglePlay(); }
  if (e.key === "Escape")     selectNode(null);
});

// Collapse/expand sidebar sections
document.querySelectorAll(".sec-h").forEach(h => {
  h.addEventListener("click", () => {
    h.classList.toggle("collapsed");
    h.nextElementSibling.classList.toggle("hidden");
  });
});

// ─────────────────────────────────────────────────────────────────────────────
// 9. Boot — load whatever state the server already has (e.g. after page refresh)
// ─────────────────────────────────────────────────────────────────────────────

fetch(`${SERVER}/state`)
  .then(r => r.json())
  .then(s => { if (s.n > 0) { pushHistory(s); applyState(s); } })
  .catch(() => { /* server not up yet */ });
