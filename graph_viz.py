#!/usr/bin/env python3
"""
Graph Visualizer — single file
================================
Usage:
    python3 graph_viz.py --binary ./your_program   # connect to your C++ binary
    python3 graph_viz.py --demo                    # demo mode, no binary needed

Install deps:  pip install flask flask-cors

Your C++ binary must:
  1. Read a newline from stdin at the start of each iteration  (waitForStep)
  2. Print a JSON line to stdout at the end of each iteration  (printState)

JSON format (one line, then newline):
  {"iteration":0,"n":8,"matrix":[[0,1,...,2],[...]],"colors":[0,1,...],"status":"running"}
  - matrix is n x (n+1); last column = degree of node i
  - status: "running" | "done"

C++ helpers to paste into your code:
─────────────────────────────────────
#include <iostream>
#include <vector>
#include <sstream>

void waitForStep() {
    std::string line;
    std::getline(std::cin, line);
}

void printState(const std::vector<std::vector<int>>& matrix,
                const std::vector<int>& colors,
                int iteration, int n,
                const std::string& status = "running") {
    std::ostringstream j;
    j << "{\"iteration\":" << iteration << ",\"n\":" << n << ",\"matrix\":[";
    for (int i = 0; i < n; i++) {
        j << "[";
        for (int k = 0; k <= n; k++) { j << matrix[i][k]; if (k < n) j << ","; }
        j << "]"; if (i < n-1) j << ",";
    }
    j << "],\"colors\":[";
    for (int i = 0; i < n; i++) { j << colors[i]; if (i < n-1) j << ","; }
    j << "],\"status\":\"" << status << "\"}";
    std::cout << j.str() << "\\n";
    std::cout.flush();
}

// In your loop:
// for (int i = 0; i < total; i++) {
//     waitForStep();
//     /* ... your code ... */
//     printState(matrix, colors, i, n, i==total-1 ? "done" : "running");
// }
─────────────────────────────────────
"""

import argparse, json, math, random, subprocess, sys, threading, webbrowser
from http.server import BaseHTTPRequestHandler, HTTPServer

# ── State ──────────────────────────────────────────────────────────────────────
process      = None
state_lock   = threading.Lock()
demo_iter    = 0
args_global  = None
current_state = {
    "iteration": -1, "n": 0, "matrix": [], "colors": [],
    "status": "idle", "message": "Press STEP to begin"
}

# ── Demo ───────────────────────────────────────────────────────────────────────
def make_demo(iteration, n=9):
    random.seed(iteration * 31 + 13)
    matrix = [[0]*(n+1) for _ in range(n)]
    for i in range(n):
        for j in range(i+1, n):
            if random.random() < 0.35 + 0.12*math.sin(iteration*0.7+i+j):
                matrix[i][j] = matrix[j][i] = 1
    for i in range(n):
        matrix[i][n] = sum(matrix[i][:n])
    colors = [(i * 3 + iteration) % 5 for i in range(n)]
    return {"iteration": iteration, "n": n, "matrix": matrix, "colors": colors,
            "status": "running" if iteration < 19 else "done",
            "message": f"Demo iteration {iteration+1}/20"}

# ── C++ process ────────────────────────────────────────────────────────────────
def start_process(binary):
    global process
    try:
        process = subprocess.Popen([binary], stdin=subprocess.PIPE,
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                   text=True, bufsize=1)
        print(f"[viz] Started: {binary} (PID {process.pid})")
    except FileNotFoundError:
        print(f"[viz] ERROR: binary not found: {binary}"); sys.exit(1)

def send_step():
    if process and process.poll() is None:
        try: process.stdin.write("\n"); process.stdin.flush(); return True
        except BrokenPipeError: pass
    return False

def read_state():
    global current_state
    if not process or process.poll() is not None:
        with state_lock: current_state["status"] = "done"; return
    try:
        line = process.stdout.readline().strip()
        if line:
            with state_lock: current_state.update(json.loads(line))
    except Exception as e:
        print(f"[viz] read error: {e}")

# ── HTTP server ────────────────────────────────────────────────────────────────
class Handler(BaseHTTPRequestHandler):
    def log_message(self, *a): pass   # silence access log

    def _json(self, data, code=200):
        body = json.dumps(data).encode()
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", len(body))
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers(); self.wfile.write(body)

    def _html(self, body):
        b = body.encode()
        self.send_response(200)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", len(b))
        self.end_headers(); self.wfile.write(b)

    def do_OPTIONS(self):
        self.send_response(204)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET,POST,OPTIONS")
        self.end_headers()

    def do_GET(self):
        if self.path in ("/", "/index.html"):
            self._html(HTML)
        elif self.path == "/state":
            with state_lock: self._json(current_state)
        else:
            self.send_response(404); self.end_headers()

    def do_POST(self):
        global demo_iter, process, current_state
        if self.path == "/step":
            if args_global.demo:
                s = make_demo(demo_iter)
                with state_lock: current_state.update(s)
                demo_iter = (demo_iter + 1) % 20
            else:
                if process and process.poll() is not None:
                    with state_lock: current_state["status"] = "done"
                else:
                    send_step(); read_state()
            with state_lock: self._json(current_state)

        elif self.path == "/reset":
            demo_iter = 0
            if process:
                process.terminate(); process = None
            with state_lock:
                current_state.update({"iteration":-1,"n":0,"matrix":[],"colors":[],
                                       "status":"idle","message":"Reset. Press STEP."})
            if args_global.binary and not args_global.demo:
                start_process(args_global.binary)
            with state_lock: self._json(current_state)
        else:
            self.send_response(404); self.end_headers()

# ── Embedded HTML ──────────────────────────────────────────────────────────────
HTML = r"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8"/>
<title>Graph Visualizer</title>
<script src="https://cdnjs.cloudflare.com/ajax/libs/d3/7.8.5/d3.min.js"></script>
<link href="https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@400;600&family=Syne:wght@800&display=swap" rel="stylesheet">
<style>
:root{
  --bg:#080b12;--surface:#0e1320;--panel:#0d1525;--border:#1a2640;
  --text:#b8ccec;--muted:#3d5575;--accent:#3b82f6;--accent2:#06b6d4;
  --done:#10b981;--c0:#3b82f6;--c1:#10b981;--c2:#f59e0b;--c3:#ec4899;
  --c4:#8b5cf6;--c5:#06b6d4;--c6:#f97316;--c7:#a3e635;--c8:#14b8a6;--c9:#e11d48;
}
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:'JetBrains Mono',monospace;background:var(--bg);color:var(--text);
     height:100vh;display:grid;grid-template-rows:48px 1fr;grid-template-columns:1fr 300px;overflow:hidden}

/* Header */
header{grid-column:1/-1;background:var(--surface);border-bottom:1px solid var(--border);
       display:flex;align-items:center;gap:16px;padding:0 16px}
.logo{font-family:'Syne',sans-serif;font-weight:800;font-size:14px;color:#fff;
      display:flex;align-items:center;gap:7px;letter-spacing:.06em}
.logo-dot{width:7px;height:7px;border-radius:50%;background:var(--accent);
          box-shadow:0 0 8px var(--accent);animation:pulse 2s ease-in-out infinite}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.3}}
.sep{width:1px;height:20px;background:var(--border);flex-shrink:0}
.pill{font-size:10px;color:var(--muted);white-space:nowrap}
.pill b{color:var(--text);font-weight:600}
.spacer{flex:1}
.badge{display:flex;align-items:center;gap:5px;font-size:10px;padding:3px 9px;
       border-radius:20px;border:1px solid var(--border);background:rgba(255,255,255,.02)}
.bdot{width:5px;height:5px;border-radius:50%;background:var(--muted)}
.badge.running .bdot{background:var(--accent);box-shadow:0 0 5px var(--accent);animation:pulse .8s infinite}
.badge.done .bdot{background:var(--done);box-shadow:0 0 5px var(--done)}
.btn{display:flex;align-items:center;gap:6px;padding:6px 13px;border:1px solid var(--border);
     border-radius:5px;background:rgba(255,255,255,.02);color:var(--text);font-family:inherit;
     font-size:10px;font-weight:600;cursor:pointer;letter-spacing:.05em;transition:all .15s}
.btn:hover{background:rgba(59,130,246,.1);border-color:var(--accent);color:#fff}
.btn:active{transform:scale(.97)}
.btn.primary{background:var(--accent);border-color:var(--accent);color:#fff;
             box-shadow:0 0 14px rgba(59,130,246,.35)}
.btn.primary:hover{background:#2563eb}
.btn.primary:disabled{opacity:.35;cursor:not-allowed;transform:none}
kbd{font-family:inherit;font-size:9px;padding:1px 4px;border-radius:3px;
    background:rgba(255,255,255,.08);border:1px solid var(--border);color:var(--muted)}

/* Graph panel */
#gpanel{position:relative;background:var(--bg);overflow:hidden}
#gsvg{width:100%;height:100%}
.link{stroke:#1e2d45;stroke-opacity:.8;transition:stroke .3s,stroke-opacity .3s}
.link.hi{stroke:var(--accent2);stroke-opacity:1;stroke-width:2.5px !important}
.node-circle{stroke-width:2;transition:r .4s}
.node-label{font-family:'JetBrains Mono',monospace;font-size:10px;fill:#fff;
            dominant-baseline:middle;text-anchor:middle;pointer-events:none;font-weight:600}
.node-deg{font-family:'JetBrains Mono',monospace;font-size:8px;fill:var(--muted);
          dominant-baseline:middle;text-anchor:middle;pointer-events:none}
.hint{position:absolute;bottom:12px;left:14px;font-size:10px;color:var(--muted);
      pointer-events:none;display:flex;gap:12px}
#empty{position:absolute;inset:0;display:flex;flex-direction:column;align-items:center;
       justify-content:center;gap:10px;pointer-events:none}
#empty svg{opacity:.25}
#empty p{font-size:11px;color:var(--muted);text-align:center;max-width:240px;line-height:1.8}

/* Side panel */
#side{background:var(--panel);border-left:1px solid var(--border);
      display:flex;flex-direction:column;overflow:hidden}
.sec{border-bottom:1px solid var(--border);flex-shrink:0}
.sec-h{display:flex;align-items:center;justify-content:space-between;
        padding:10px 14px;font-size:9px;font-weight:600;letter-spacing:.12em;
        color:var(--muted);text-transform:uppercase;cursor:pointer;user-select:none}
.sec-h:hover{color:var(--text)}
.sec-h .arr{transition:transform .2s}
.sec-h.col .arr{transform:rotate(-90deg)}
.sec-body{padding:10px 14px}
.sec-body.hide{display:none}

/* Stats */
.sgrid{display:grid;grid-template-columns:1fr 1fr;gap:6px}
.scard{background:rgba(255,255,255,.02);border:1px solid var(--border);
       border-radius:5px;padding:8px 10px}
.scard-l{font-size:9px;color:var(--muted);letter-spacing:.1em;text-transform:uppercase}
.scard-v{font-size:16px;font-weight:600;color:#fff;margin-top:1px}

/* Inspector */
.irow{display:flex;justify-content:space-between;align-items:center;
      padding:4px 0;font-size:10px;border-bottom:1px solid rgba(255,255,255,.04)}
.irow:last-child{border:none}
.il{color:var(--muted)}.iv{color:#fff;font-weight:500}
.swatch{display:inline-block;width:8px;height:8px;border-radius:50%;margin-right:4px;vertical-align:middle}
.chips{display:flex;flex-wrap:wrap;gap:4px;margin-top:6px}
.chip{font-size:9px;padding:2px 7px;border-radius:10px;border:1px solid var(--border);color:var(--muted)}

/* Color legend */
.leg{display:flex;flex-direction:column;gap:5px}
.leg-row{display:flex;align-items:center;gap:7px;font-size:10px}
.leg-dot{width:7px;height:7px;border-radius:50%;flex-shrink:0}
.leg-cnt{margin-left:auto;color:var(--muted);font-size:9px}

/* Node list */
.nlist{overflow-y:auto;flex:1;padding:4px 8px 8px}
.nrow{display:flex;align-items:center;gap:7px;padding:4px 6px;border-radius:4px;
      font-size:10px;cursor:pointer;transition:background .12s;margin-bottom:1px}
.nrow:hover{background:rgba(255,255,255,.04)}
.nrow.active{background:rgba(59,130,246,.1)}
.nrow-id{color:var(--muted);width:18px;font-size:9px}
.nrow-dot{width:7px;height:7px;border-radius:50%;flex-shrink:0}
.nrow-name{flex:1}
.nrow-deg{color:var(--muted);font-size:9px}

/* Matrix */
.mscroll{overflow:auto;max-height:180px}
.mtbl{border-collapse:collapse;font-size:9px}
.mtbl th,.mtbl td{width:20px;height:20px;text-align:center;
                  border:1px solid rgba(255,255,255,.04)}
.mtbl th{color:var(--muted);font-weight:400;background:rgba(255,255,255,.02)}
.mtbl td.e1{background:rgba(59,130,246,.2);color:var(--accent)}
.mtbl td.deg{background:rgba(255,255,255,.02);color:var(--muted);border-left:1px solid var(--border)}
.mtbl td.self{background:rgba(255,255,255,.015)}
.mtbl td.hrow{background:rgba(59,130,246,.06)}

::-webkit-scrollbar{width:3px;height:3px}
::-webkit-scrollbar-thumb{background:var(--border);border-radius:2px}
</style>
</head>
<body>

<header>
  <div class="logo"><div class="logo-dot" id="ldot"></div>GRAPH VIZ</div>
  <div class="sep"></div>
  <div class="pill">ITER <b id="hIter">—</b></div>
  <div class="pill">NODES <b id="hNodes">—</b></div>
  <div class="pill">EDGES <b id="hEdges">—</b></div>
  <div class="sep"></div>
  <div class="badge idle" id="badge"><div class="bdot"></div><span id="btext">Idle</span></div>
  <div class="spacer"></div>
  <button class="btn" id="fitBtn" title="F">
    <svg width="11" height="11" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5">
      <path d="M8 3H5a2 2 0 00-2 2v3m18 0V5a2 2 0 00-2-2h-3m0 18h3a2 2 0 002-2v-3M3 16v3a2 2 0 002 2h3"/>
    </svg>FIT
  </button>
  <button class="btn" id="resetBtn">
    <svg width="11" height="11" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5">
      <polyline points="1 4 1 10 7 10"/><path d="M3.51 15a9 9 0 102.13-9.36L1 10"/>
    </svg>RESET
  </button>
  <button class="btn primary" id="stepBtn">
    <svg width="11" height="11" viewBox="0 0 24 24" fill="currentColor"><polygon points="5,3 19,12 5,21"/></svg>
    STEP <kbd>↵</kbd>
  </button>
</header>

<div id="gpanel">
  <svg id="gsvg"></svg>
  <div id="empty">
    <svg width="52" height="52" viewBox="0 0 24 24" fill="none" stroke="#4a6080" stroke-width="1.2">
      <circle cx="5" cy="12" r="3"/><circle cx="19" cy="5" r="3"/><circle cx="19" cy="19" r="3"/>
      <line x1="8" y1="12" x2="16" y2="6"/><line x1="8" y1="12" x2="16" y2="18"/>
    </svg>
    <p>Press <strong>STEP</strong> or <kbd>Enter</kbd><br>to run the first iteration.</p>
  </div>
  <div class="hint"><span>Scroll=zoom</span><span>Drag=pan</span><span>Click=inspect</span></div>
</div>

<div id="side">
  <!-- Stats -->
  <div class="sec">
    <div class="sec-h" onclick="tog(this)">Graph Stats<span class="arr">▾</span></div>
    <div class="sec-body" id="sStats">
      <div class="sgrid">
        <div class="scard"><div class="scard-l">Visible</div><div class="scard-v" id="sVis">0</div></div>
        <div class="scard"><div class="scard-l">Edges</div><div class="scard-v" id="sEdge">0</div></div>
        <div class="scard"><div class="scard-l">Max Deg</div><div class="scard-v" id="sMaxD">0</div></div>
        <div class="scard"><div class="scard-l">Colors</div><div class="scard-v" id="sCol">0</div></div>
      </div>
    </div>
  </div>

  <!-- Inspector -->
  <div class="sec">
    <div class="sec-h" onclick="tog(this)">Node Inspector<span class="arr">▾</span></div>
    <div class="sec-body" id="sInsp">
      <div id="iEmpty" style="font-size:10px;color:var(--muted);text-align:center;padding:8px 0">Click a node</div>
      <div id="iDetail" style="display:none"></div>
    </div>
  </div>

  <!-- Colors -->
  <div class="sec">
    <div class="sec-h" onclick="tog(this)">Colors<span class="arr">▾</span></div>
    <div class="sec-body" id="sColors"><div class="leg" id="legend"></div></div>
  </div>

  <!-- Node list -->
  <div class="sec" style="flex:1;overflow:hidden;display:flex;flex-direction:column;min-height:0">
    <div class="sec-h" onclick="tog(this)">Active Nodes<span class="arr">▾</span></div>
    <div class="nlist sec-body" id="sNodes" style="overflow-y:auto;padding:6px 10px"></div>
  </div>

  <!-- Matrix -->
  <div class="sec" style="flex-shrink:0">
    <div class="sec-h" onclick="tog(this)">Adjacency Matrix<span class="arr">▾</span></div>
    <div class="sec-body" id="sMatrix"><div class="mscroll"><table class="mtbl" id="mtbl"></table></div></div>
  </div>
</div>

<script>
const COLORS = ['#3b82f6','#10b981','#f59e0b','#ec4899','#8b5cf6',
                '#06b6d4','#f97316','#a3e635','#14b8a6','#e11d48'];
const col = i => COLORS[i % COLORS.length];

let gs = {n:0,matrix:[],colors:[],iteration:-1,status:'idle'};
let selNode = null, nodePosCache = {}, sim = null, busy = false;

const svg    = d3.select('#gsvg');
const gpanel = document.getElementById('gpanel');
const zoom   = d3.zoom().scaleExtent([.1,8]).on('zoom', e => root.attr('transform', e.transform));
svg.call(zoom);
svg.append('defs').append('pattern').attr('id','gr').attr('width',40).attr('height',40)
   .attr('patternUnits','userSpaceOnUse')
   .append('path').attr('d','M40 0L0 0 0 40').attr('fill','none')
   .attr('stroke','#0d1525').attr('stroke-width','1');
svg.append('rect').attr('width','100%').attr('height','100%').attr('fill','url(#gr)');
const root  = svg.append('g');
const lL    = root.append('g');
const nL    = root.append('g');

// Build nodes/links from matrix (degree>0 only)
function buildGraph({n,matrix,colors}) {
  const nodes=[],links=[],seen=new Set();
  for(let i=0;i<n;i++) if(matrix[i][n]>0) nodes.push({id:i,color:colors[i],degree:matrix[i][n]});
  const active=new Set(nodes.map(d=>d.id));
  for(let i=0;i<n;i++){if(!active.has(i))continue;
    for(let j=i+1;j<n;j++){if(!active.has(j))continue;
      if(matrix[i][j]){const k=`${i}-${j}`;if(!seen.has(k)){seen.add(k);links.push({source:i,target:j});}}}}
  return {nodes,links};
}

function render(state) {
  const {nodes,links} = buildGraph(state);
  const W=gpanel.clientWidth, H=gpanel.clientHeight;
  document.getElementById('empty').style.display = nodes.length?'none':'flex';

  // Save positions
  nL.selectAll('.ng').each(d=>{nodePosCache[d.id]={x:d.x,y:d.y};});

  nodes.forEach(n=>{
    if(nodePosCache[n.id]){n.x=nodePosCache[n.id].x;n.y=nodePosCache[n.id].y;}
    else{n.x=W/2+(Math.random()-.5)*180;n.y=H/2+(Math.random()-.5)*180;}
  });

  if(sim) sim.stop();
  sim = d3.forceSimulation(nodes)
    .force('link',   d3.forceLink(links).id(d=>d.id).distance(85).strength(.5))
    .force('charge', d3.forceManyBody().strength(-200))
    .force('center', d3.forceCenter(W/2,H/2))
    .force('collide',d3.forceCollide(26))
    .alphaDecay(.04);

  // Links
  const lSel = lL.selectAll('.link').data(links, d=>`${d.source.id??d.source}-${d.target.id??d.target}`);
  lSel.enter().append('line').attr('class','link').attr('stroke-width',1.5).merge(lSel);
  lSel.exit().remove();

  // Nodes
  const nSel = nL.selectAll('.ng').data(nodes, d=>d.id);
  const entered = nSel.enter().append('g').attr('class','ng')
    .call(d3.drag()
      .on('start',(e,d)=>{if(!e.active)sim.alphaTarget(.3).restart();d.fx=d.x;d.fy=d.y;})
      .on('drag', (e,d)=>{d.fx=e.x;d.fy=e.y;})
      .on('end',  (e,d)=>{if(!e.active)sim.alphaTarget(0);d.fx=null;d.fy=null;}))
    .on('click',(e,d)=>{e.stopPropagation();inspect(d,state);});

  entered.append('circle').attr('class','node-circle');
  entered.append('text').attr('class','node-label');
  entered.append('text').attr('class','node-deg').attr('dy',22);
  nSel.exit().remove();

  const all = entered.merge(nSel);
  all.select('.node-circle')
     .attr('r', d=>11+Math.min(d.degree*1.4,7))
     .attr('fill',d=>col(d.color)).attr('stroke',d=>col(d.color))
     .style('filter',d=>`drop-shadow(0 0 5px ${col(d.color)}88)`);
  all.select('.node-label').text(d=>d.id);
  all.select('.node-deg').text(d=>`d=${d.degree}`);

  sim.on('tick',()=>{
    lL.selectAll('.link')
      .attr('x1',d=>d.source.x).attr('y1',d=>d.source.y)
      .attr('x2',d=>d.target.x).attr('y2',d=>d.target.y);
    nL.selectAll('.ng').attr('transform',d=>`translate(${d.x},${d.y})`);
  });

  updatePanel(state,nodes,links);
}

function updatePanel(state,nodes,links) {
  const {n,matrix,colors}=state;
  document.getElementById('hIter').textContent  = state.iteration>=0?state.iteration:'—';
  document.getElementById('hNodes').textContent = n||'—';
  document.getElementById('hEdges').textContent = links.length||'—';
  document.getElementById('sVis').textContent   = nodes.length;
  document.getElementById('sEdge').textContent  = links.length;
  document.getElementById('sMaxD').textContent  = nodes.length?Math.max(...nodes.map(d=>d.degree)):0;
  document.getElementById('sCol').textContent   = new Set(nodes.map(d=>d.color)).size;

  // Color legend
  const cc={};
  nodes.forEach(d=>{cc[d.color]=(cc[d.color]||0)+1;});
  document.getElementById('legend').innerHTML = Object.keys(cc).length
    ? Object.entries(cc).sort((a,b)=>+a[0]-+b[0]).map(([ci,cnt])=>
        `<div class="leg-row"><div class="leg-dot" style="background:${col(+ci)}"></div>
         <span>Color ${ci}</span><span class="leg-cnt">${cnt}×</span></div>`).join('')
    : '<div style="font-size:10px;color:var(--muted)">No data</div>';

  // Node list
  document.getElementById('sNodes').innerHTML = nodes.map(nd=>
    `<div class="nrow${nd.id===selNode?' active':''}" onclick="selById(${nd.id})">
       <span class="nrow-id">${nd.id}</span>
       <span class="nrow-dot" style="background:${col(nd.color)}"></span>
       <span class="nrow-name">Node ${nd.id}</span>
       <span class="nrow-deg">d=${nd.degree}</span>
     </div>`).join('');

  buildMatrix(state,nodes);
}

function buildMatrix(state,visNodes) {
  const {n,matrix}=state; const ids=visNodes.map(d=>d.id);
  if(!ids.length){document.getElementById('mtbl').innerHTML='<tr><td style="color:var(--muted);padding:8px;font-size:10px">No active nodes</td></tr>';return;}
  let h=`<thead><tr><th></th>${ids.map(i=>`<th>${i}</th>`).join('')}<th style="color:var(--accent2)">d</th></tr></thead><tbody>`;
  ids.forEach(i=>{
    h+=`<tr><th>${i}</th>`;
    ids.forEach(j=>{
      const v=matrix[i][j];
      const c=i===j?'self':(v?'e1':'')+(i===selNode?' hrow':'');
      h+=`<td class="${c}">${i===j?'·':v}</td>`;
    });
    h+=`<td class="deg">${matrix[i][n]}</td></tr>`;
  });
  h+='</tbody>';
  document.getElementById('mtbl').innerHTML=h;
}

function inspect(nd, state) {
  selNode=nd.id;
  // edge highlights
  lL.selectAll('.link').classed('hi',d=>d.source.id===nd.id||d.target.id===nd.id)
    .attr('stroke-width',d=>(d.source.id===nd.id||d.target.id===nd.id)?2.5:1.5);

  const {n,matrix}=state;
  const nbrs=[];
  for(let j=0;j<n;j++) if(j!==nd.id&&matrix[nd.id][j]) nbrs.push(j);

  document.getElementById('iEmpty').style.display='none';
  document.getElementById('iDetail').style.display='block';
  document.getElementById('iDetail').innerHTML=`
    <div class="irow"><span class="il">ID</span><span class="iv">${nd.id}</span></div>
    <div class="irow"><span class="il">Degree</span><span class="iv">${nd.degree}</span></div>
    <div class="irow"><span class="il">Color</span><span class="iv">
      <span class="swatch" style="background:${col(nd.color)}"></span>${nd.color}</span></div>
    <div class="irow"><span class="il">Neighbors</span><span class="iv">${nbrs.length}</span></div>
    <div class="chips">${nbrs.map(j=>`<span class="chip">${j}</span>`).join('')}</div>`;

  // refresh matrix highlight + node list
  const {nodes,links}=buildGraph(state);
  buildMatrix(state,nodes);
  document.querySelectorAll('.nrow').forEach(el=>{
    const id=parseInt(el.textContent.match(/\d+/)?.[0]);
    el.classList.toggle('active',id===nd.id);
  });
}

function selById(id) {
  const {nodes}=buildGraph(gs);
  const nd=nodes.find(n=>n.id===id);
  if(nd) inspect(nd,gs);
}

function setStatus(s,msg){
  const b=document.getElementById('badge');
  b.className='badge '+s;
  document.getElementById('btext').textContent=msg;
  document.getElementById('ldot').style.animationPlayState=s==='running'?'running':'paused';
}

async function step(){
  if(busy)return;
  busy=true;
  document.getElementById('stepBtn').disabled=true;
  setStatus('running','Stepping…');
  try{
    const r=await fetch('/step',{method:'POST'});
    if(!r.ok) throw new Error(await r.text());
    gs=await r.json();
    render(gs);
    if(gs.status==='done'){setStatus('done','Done');}
    else{setStatus('running',gs.message||`Iter ${gs.iteration}`);document.getElementById('stepBtn').disabled=false;}
  }catch(e){setStatus('idle','Error: '+e.message);document.getElementById('stepBtn').disabled=false;}
  finally{busy=false;}
}

async function reset(){
  document.getElementById('stepBtn').disabled=false;
  selNode=null; nodePosCache={};
  nL.selectAll('*').remove(); lL.selectAll('*').remove();
  document.getElementById('empty').style.display='flex';
  document.getElementById('iEmpty').style.display='block';
  document.getElementById('iDetail').style.display='none';
  setStatus('idle','Reset');
  await fetch('/reset',{method:'POST'});
  gs={n:0,matrix:[],colors:[],iteration:-1,status:'idle'};
}

function fitView(){
  const b=root.node().getBBox();
  const W=gpanel.clientWidth,H=gpanel.clientHeight;
  if(!b.width||!b.height)return;
  const s=Math.min(.88*W/b.width,.88*H/b.height,2.5);
  svg.transition().duration(450).call(zoom.transform,
    d3.zoomIdentity.translate(W/2-s*(b.x+b.width/2),H/2-s*(b.y+b.height/2)).scale(s));
}

function tog(h){
  h.classList.toggle('col');
  h.nextElementSibling.classList.toggle('hide');
}

document.addEventListener('keydown',e=>{
  if(e.key==='Enter'&&document.activeElement.tagName!=='INPUT'){e.preventDefault();step();}
  if(e.key==='f'||e.key==='F') fitView();
  if(e.key==='Escape'){selNode=null;lL.selectAll('.link').classed('hi',false).attr('stroke-width',1.5);render(gs);}
});

svg.on('click',()=>{
  selNode=null;
  lL.selectAll('.link').classed('hi',false).attr('stroke-width',1.5);
  document.getElementById('iEmpty').style.display='block';
  document.getElementById('iDetail').style.display='none';
});

document.getElementById('stepBtn').addEventListener('click',step);
document.getElementById('resetBtn').addEventListener('click',reset);
document.getElementById('fitBtn').addEventListener('click',fitView);

// Initial fetch
fetch('/state').then(r=>r.json()).then(d=>{if(d.n>0){gs=d;render(d);}}).catch(()=>{});
</script>
</body>
</html>"""

# ── Entry point ────────────────────────────────────────────────────────────────
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Graph Visualizer")
    parser.add_argument("--binary", type=str, help="Path to C++ binary")
    parser.add_argument("--port",   type=int, default=5000)
    parser.add_argument("--demo",   action="store_true", help="Demo mode (no binary needed)")
    args = parser.parse_args()
    args_global = args

    if not args.demo and not args.binary:
        print("Usage: python3 graph_viz.py --binary ./your_program\n"
              "       python3 graph_viz.py --demo")
        sys.exit(1)

    if args.binary and not args.demo:
        start_process(args.binary)

    url = f"http://localhost:{args.port}"
    print(f"[viz] Mode : {'DEMO' if args.demo else args.binary}")
    print(f"[viz] Open : {url}")

    threading.Timer(0.8, lambda: webbrowser.open(url)).start()

    server = HTTPServer(("", args.port), Handler)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\n[viz] Stopped.")
        if process: process.terminate()
