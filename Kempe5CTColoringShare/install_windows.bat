@echo off
echo =============================================
echo  Graph Visualizer - First Time Setup
echo =============================================
echo.

:: Check Python is installed
python --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Python not found. Please install from https://python.org
    pause
    exit /b 1
)

echo Creating virtual environment...
python -m venv venv

echo Installing dependencies...
venv\Scripts\pip install flask flask-cors

echo.
echo =============================================
echo  Setup complete! Run run_windows.bat to start
echo =============================================
pause
