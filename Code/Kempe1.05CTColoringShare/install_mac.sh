#!/bin/bash
echo "============================================="
echo " Graph Visualizer - First Time Setup (Mac)"
echo "============================================="

# Check Python
if ! command -v python3 &> /dev/null; then
    echo "ERROR: Python3 not found. Install from https://python.org"
    exit 1
fi

# Check g++ (Xcode Command Line Tools)
if ! command -v g++ &> /dev/null; then
    echo "g++ not found. Installing Xcode Command Line Tools..."
    xcode-select --install
    echo "After installation completes, run this script again."
    exit 1
fi

echo "Compiling C++ program..."
g++ -O2 -std=c++17 -o KempeBased5CT KempeBased5CT.cpp
if [ $? -ne 0 ]; then
    echo "ERROR: Compilation failed."
    exit 1
fi

echo "Creating virtual environment..."
python3 -m venv venv

echo "Installing dependencies..."
venv/bin/pip install flask flask-cors

echo ""
echo "============================================="
echo " Setup complete! Run: bash run_mac.sh"
echo "============================================="
