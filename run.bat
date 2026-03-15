@echo off
echo Starting Graph Visualizer...
start "" http://localhost:5000
python server.py --binary ./program.exe
pause