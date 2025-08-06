#!/bin/bash
# Setup Analysis Environment for Stage 6

echo "=== Setting Up Analysis Environment ==="
echo "======================================"

echo ""
echo "1. Checking Python installation..."
if ! command -v python3 &> /dev/null; then
    echo "❌ Python 3 not found"
    echo "Please install Python 3 first:"
    echo "  Ubuntu/Debian: sudo apt install python3 python3-pip"
    echo "  macOS: brew install python"
    echo "  Windows: Download from https://python.org"
    exit 1
fi

echo "✅ Python 3 found: $(python3 --version)"

echo ""
echo "2. Installing Python dependencies..."
if [ -f "analysis/requirements.txt" ]; then
    echo "Installing from requirements.txt..."
    if python3 -m pip install -r analysis/requirements.txt; then
        echo "✅ Python dependencies installed successfully"
    else
        echo "⚠️ Some dependencies failed to install"
        echo "You may need to install them manually"
    fi
else
    echo "Installing essential packages manually..."
    python3 -m pip install pandas numpy matplotlib seaborn scikit-learn
fi

echo ""
echo "3. Testing Python environment..."
python3 -c "
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
print('✅ All essential packages working')
print(f'Pandas: {pd.__version__}')
print(f'NumPy: {np.__version__}')
print(f'Matplotlib: {plt.matplotlib.__version__}')
print(f'Seaborn: {sns.__version__}')
"

echo ""
echo "4. Creating output directories..."
mkdir -p analysis/outputs
mkdir -p results

echo ""
echo "5. Testing analysis scripts..."
if [ -f "results/sample/test_experiment.csv" ]; then
    echo "Testing with sample data..."
    if python3 analysis/scripts/precision_recall_analysis.py results/sample --output analysis/outputs; then
        echo "✅ Analysis script test successful"

        if python3 analysis/scripts/generate_report.py results/sample --output analysis/outputs; then
            echo "✅ Report generation test successful"

            if [ -f "analysis/outputs/analysis_report.html" ]; then
                echo "📄 Test report generated: analysis/outputs/analysis_report.html"
            fi
        fi
    fi
else
    echo "⚠️ No sample data found - run validate_stage6.sh first"
fi

echo ""
echo "=== Analysis Environment Setup Complete ==="
echo ""
echo "✅ Your analysis environment is ready!"
echo ""
echo "Quick Start:"
echo "  1. Run experiments: cd build && ./descriptor_compare"
echo "  2. Analyze results: make run_analysis"
echo "  3. View reports: open analysis/outputs/analysis_report.html"
echo ""
echo "Manual Usage:"
echo "  python3 analysis/scripts/precision_recall_analysis.py results/"
echo "  python3 analysis/scripts/generate_report.py results/"
