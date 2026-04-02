=============================================
 Kempe-Based Method for 5CT — Graph Visualizer
=============================================

WINDOWS
-------
1. Double-click install_windows.bat   (first time only)
2. Double-click run_windows.bat       (every time)

MAC
---
1. Open Terminal, navigate to this folder
2. Run: bash install_mac.sh           (first time only)
3. Run: bash run_mac.sh               (every time)

REQUIREMENTS
------------
- c++ 17 installed (if on MAC, xcode should have Clang which contains c++17)
- Python 3 (latest version; check if your current version doesn't work first)  (https://python.org)
- Mac only: Xcode Command Line Tools (install_mac.sh handles this)

USAGE
-----
- Click "LOAD GRAPH" to paste an adjacency matrix. copy and paste contents in "usa states.txt" into the "LOAD GRAPH" space.
- Press STEP (or Enter / Space / →) to advance one iteration
- Press BACK (or ←) to go back through history
- Press PLAY to auto-step through all iterations
- Click any node to inspect its color, degree, and neighbors
- Drag nodes to reposition them
- Scroll to zoom, drag canvas to pan
- Click "Export state as JSON" to save the current state

NOTES
-----
- The matrix must be square, symmetric, and contain only 0s and 1s
- n is detected automatically from the matrix dimensions
- Nodes with degree 0 are hidden from the graph view
- Colors: 0=uncolored, 1=red, 2=green, 3=blue, 4=yellow, 5=white
