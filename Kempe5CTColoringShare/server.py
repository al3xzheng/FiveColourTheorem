"""
server.py
─────────
Run:  python server.py --binary ./program.exe
      python server.py --demo
Deps: pip install flask flask-cors

C++ expected flow:
  1. viz::loadGraph()   reads graph JSON from stdin
  2. viz::printState()  prints ONCE immediately (server reads this in /load)
  3. loop: viz::waitForStep() → work → viz::printState()   (each /step triggers one)
"""

import argparse, json, math, random, subprocess, sys, threading, time, webbrowser, os
from flask import Flask, jsonify, request, send_from_directory
from flask_cors import CORS

FRONTEND = os.path.join(os.path.dirname(os.path.abspath(__file__)), "frontend")
app = Flask(__name__, static_folder=FRONTEND)
CORS(app)

_proc    = None
_lock    = threading.Lock()
_pending = None

_state = {
    "iteration": -1, "n": 0, "matrix": [], "colors": [],
    "status": "idle", "message": "Load a graph to begin.",
}

def _get():
    with _lock: return dict(_state)

def _set(u):
    with _lock: _state.update(u)

def _alive():
    return _proc is not None and _proc.poll() is None

def _spawn(binary):
    global _proc
    _proc = subprocess.Popen(
        [binary],
        stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        text=True, bufsize=1,
    )
    print(f"[server] spawned pid={_proc.pid}")
    # Forward C++ stderr to our terminal in a background thread
    def _pipe_stderr():
        for line in _proc.stderr:
            print(f"[cpp] {line}", end="")
    threading.Thread(target=_pipe_stderr, daemon=True).start()

def _send(text):
    """Write one line to C++ stdin."""
    if not _alive(): return False
    try:
        _proc.stdin.write(text + "\n")
        _proc.stdin.flush()
        return True
    except BrokenPipeError:
        return False

def _read():
    """
    Read one JSON line from C++ stdout.
    Blocks until C++ calls printState() — intentional.
    Safe because Flask runs threaded=True.
    """
    if not _alive():
        _set({"status": "done", "message": "Process finished."})
        return
    try:
        line = _proc.stdout.readline().strip()
        print(f"[server] read: {line[:120]}")
        if line:
            _set(json.loads(line))
        else:
            _set({"status": "done", "message": "Process finished."})
    except Exception as e:
        print(f"[server] read error: {e}")

# ── demo ──────────────────────────────────────────────────────────────────
_demo_i = 0
def _demo_step():
    global _demo_i
    i, n = _demo_i, 10
    random.seed(i * 31 + 7)
    m = [[0] * (n + 1) for _ in range(n)]
    for r in range(n):
        for c in range(r + 1, n):
            if random.random() < 0.3 + 0.15 * math.sin(i * 0.5 + r + c):
                m[r][c] = m[c][r] = 1
    for r in range(n): m[r][n] = sum(m[r][:n])
    colors = [(r * 2 + i) % 4 for r in range(n)]
    _set({"iteration": i, "n": n, "matrix": m, "colors": colors,
          "status": "done" if i == 24 else "running",
          "message": f"Demo {i+1}/25"})
    _demo_i = (_demo_i + 1) % 25

# ── routes ────────────────────────────────────────────────────────────────

@app.get("/")
def r_index():
    return send_from_directory(FRONTEND, "index.html")

@app.get("/<path:f>")
def r_static(f):
    import mimetypes
    mime, _ = mimetypes.guess_type(f)
    return send_from_directory(FRONTEND, f, mimetype=mime or "text/plain")

@app.get("/state")
def r_state():
    return jsonify(_get())

@app.post("/step")
def r_step():
    """
    Signal C++ to advance one iteration and read its response.
    C++: waitForStep() unblocks → work → printState() → waitForStep() blocks again
    """
    if _args.demo:
        _demo_step()
    elif not _alive():
        _set({"status": "done"})
    else:
        _send("")   # unblocks viz::waitForStep()
        _read()     # waits for viz::printState() output
    return jsonify(_get())

@app.post("/reset")
def r_reset():
    global _proc, _demo_i
    _demo_i = 0
    if _proc:
        _proc.terminate()
        _proc = None
    _set({"iteration": -1, "n": 0, "matrix": [], "colors": [],
          "status": "idle", "message": "Reset. Load a graph to begin."})
    if _args.binary and not _args.demo:
        _spawn(_args.binary)
        if _pending:
            time.sleep(0.25)
            _send(json.dumps({"n": _pending["n"], "matrix": _pending["matrix"]}, separators=(",", ":")))
            # initial printState will be read on first /step call
    return jsonify(_get())

@app.post("/load")
def r_load():
    """
    Load a new graph:
      1. Kill existing process
      2. Spawn fresh process
      3. Wait for it to start
      4. Send graph JSON → C++ reads via viz::loadGraph()
      5. C++ runs setup, calls viz::printState() once before first viz::waitForStep()
      6. Read that initial state — pipe is now clear for /step calls
    """
    global _proc, _pending

    data   = request.get_json()
    n      = int(data["n"])
    matrix = data["matrix"]

    if not _args.binary or _args.demo:
        return jsonify({"error": "Not in binary mode"}), 400

    if _proc:
        _proc.terminate()
        _proc = None

    _pending = {"n": n, "matrix": matrix}

    _set({"iteration": -1, "n": 0, "matrix": [], "colors": [],
          "status": "idle", "message": "Graph loaded. Press STEP to begin."})

    _spawn(_args.binary)
    time.sleep(0.25)  # wait for process to reach viz::loadGraph()

    _send(json.dumps({"n": n, "matrix": matrix}, separators=(",", ":")))
    # Don't call _read() here — the initial printState C++ emits before the
    # first waitForStep will be read naturally on the first /step call.

    return jsonify(_get())

# ── entry ─────────────────────────────────────────────────────────────────
_args = None
if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("--binary")
    p.add_argument("--port", type=int, default=5000)
    p.add_argument("--demo", action="store_true")
    _args = p.parse_args()

    if not _args.demo and not _args.binary:
        print("Usage:\n  python server.py --binary ./program.exe\n  python server.py --demo")
        sys.exit(1)

    if _args.binary and not _args.demo:
        _spawn(_args.binary)

    url = f"http://localhost:{_args.port}"
    print(f"[server] {_args.binary or 'DEMO'} -> {url}")
    threading.Timer(0.9, lambda: webbrowser.open(url)).start()

    # threaded=True — each request gets its own thread
    # This means _read() blocking in /step does NOT freeze /load or /state
    app.run(port=_args.port, debug=False, threaded=True)
