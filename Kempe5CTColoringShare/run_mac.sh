#!/bin/bash
echo "Starting Graph Visualizer..."

# Check venv exists
if [ ! -d "venv" ]; then
    echo "ERROR: Virtual environment not found."
    echo "Please run: bash install_mac.sh"
    exit 1
fi

# Check binary exists
if [ ! -f "KempeBased5CT" ]; then
    echo "ERROR: program binary not found."
    echo "Please run: bash install_mac.sh"
    exit 1
fi

open http://localhost:5000
venv/bin/python server.py --binary ./KempeBased5CT
