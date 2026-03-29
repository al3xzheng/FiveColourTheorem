@echo off
echo Starting Graph Visualizer...

:: Check venv exists
if not exist venv\ (
    echo ERROR: Virtual environment not found.
    echo Please run install_windows.bat first.
    pause
    exit /b 1
)

:: Check binary exists
if not exist KempeBased5CT.exe (
    echo ERROR: program.exe not found.
    echo Please compile your C++ code first:
    echo   g++ -O2 -std=c++17 -o KempeBased5CT.exe KempeBased5CT.cpp
    pause
    exit /b 1
)

venv\Scripts\python server.py --binary ./KempeBased5CT.exe
pause
