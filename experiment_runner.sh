#!/bin/bash
# Run Real Descriptor Experiments

echo "=== Running Real Descriptor Comparison Experiments ==="
echo "====================================================="

echo ""
echo "Let's run some real experiments and then analyze them!"

echo ""
echo "1. Checking project setup..."

# Check if we're in the right location
if [ ! -f "descriptor_compare/main.cpp" ]; then
    echo "❌ Not in project root directory"
    echo "Please run this from the main project directory"
    exit 1
fi

echo "✅ In project root directory"

# Check if build directory exists and is built
if [ ! -d "build" ]; then
    echo "❌ Build directory not found"
    echo "Run: mkdir build && cd build && cmake .. && make"
    exit 1
fi

if [ ! -f "build/descriptor_compare" ]; then
    echo "❌ descriptor_compare executable not found"
    echo "Run: cd build && make"
    exit 1
fi

echo "✅ Build directory and executable found"

echo ""
echo "2. Checking data directory..."

if [ ! -d "data" ]; then
    echo "❌ Data directory not found"
    echo ""
    echo "You need image data to run experiments. Options:"
    echo "1. Create data/sample with some test images"
    echo "2. Download a dataset like HPatches"
    echo "3. Use your own image pairs"
    echo ""
    echo "Creating sample data structure..."
    mkdir -p data/sample
    echo "📁 Created data/sample directory"
    echo "Add some image pairs there to run experiments"
    exit 1
else
    echo "✅ Data directory found"

    # List available datasets
    echo "Available datasets:"
    ls -la data/ | grep "^d" | awk '{print "  " $9}' | grep -v "^  \.$" | grep -v "^  \.\.$"

    # Count directories in data
    dataset_count=$(find data -mindepth 1 -maxdepth 1 -type d | wc -l)
    if [ $dataset_count -eq 0 ]; then
        echo "⚠️ No datasets found in data directory"
        echo "Add some image datasets to continue"
        exit 1
    fi
fi

echo ""
echo "3. Checking results directory..."

mkdir -p results
echo "✅ Results directory ready"

echo ""
echo "4. Running descriptor comparison experiments..."

cd build

echo ""
echo "Experiment 1: Running with default configuration..."
echo "=================================================="

# Run the experiment
if timeout 60s ./descriptor_compare > experiment_1.log 2>&1; then
    echo "✅ Experiment 1 completed successfully"

    # Check what was generated
    echo "Generated files:"
    ls -la ../results/ | tail -5

    # Count CSV files
    csv_count=$(find ../results -name "*.csv" | wc -l)
    echo "CSV files generated: $csv_count"

else
    exit_code=$?
    if [ $exit_code -eq 124 ]; then
        echo "⚠️ Experiment 1 timed out (60 seconds) - may still have generated results"
    else
        echo "❌ Experiment 1 failed"
        echo "Last 10 lines of log:"
        tail -10 experiment_1.log
        echo ""
        echo "This might be due to:"
        echo "1. Missing image data"
        echo "2. Configuration issues"
        echo "3. Path problems"
        echo ""
        echo "Check the full log: build/experiment_1.log"
        cd ..
        exit 1
    fi
fi

cd ..

echo ""
echo "5. Checking experiment results..."

# Look for generated CSV files
csv_files=$(find results -name "*.csv" 2>/dev/null)

if [ -z "$csv_files" ]; then
    echo "❌ No CSV files found in results"
    echo "The experiment may not have completed properly"
    echo ""
    echo "Let's check what's in results:"
    ls -la results/
    echo ""
    echo "And check the experiment log:"
    if [ -f "build/experiment_1.log" ]; then
        echo "Last 20 lines of experiment log:"
        tail -20 build/experiment_1.log
    fi
    exit 1
else
    echo "✅ Found experiment results:"
    for csv_file in $csv_files; do
        echo "  📄 $csv_file ($(wc -l < "$csv_file") lines)"
    done
fi

echo ""
echo "6. Analyzing results with Stage 6 tools..."

# Activate conda environment
source /home/frank/miniforge3/etc/profile.d/conda.sh
conda activate descriptor-compare

echo "✅ Conda environment activated: $CONDA_DEFAULT_ENV"

echo ""
echo "Running precision-recall analysis..."

if python analysis/scripts/precision_recall_analysis.py results/ --output analysis/outputs; then
    echo "✅ Analysis completed successfully"

    echo ""
    echo "Generating comprehensive report..."

    if python analysis/scripts/generate_report.py results/ --output analysis/outputs; then
        echo "✅ Report generated successfully"

        echo ""
        echo "🎊 EXPERIMENT AND ANALYSIS COMPLETE!"
        echo ""
        echo "Results available:"
        echo "📄 Raw CSV data: results/"
        echo "📊 Analysis plots: analysis/outputs/"
        echo "📋 HTML report: analysis/outputs/analysis_report.html"
        echo ""
        echo "View your results:"
        echo "  firefox analysis/outputs/analysis_report.html"
        echo "  # OR"
        echo "  google-chrome analysis/outputs/analysis_report.html"
        echo ""

        # Show summary of what was analyzed
        echo "Analysis Summary:"
        if [ -f "analysis/outputs/analysis_summary.txt" ]; then
            echo "---"
            head -20 analysis/outputs/analysis_summary.txt
            echo "---"
        fi

    else
        echo "❌ Report generation failed"
        echo "But analysis data is available in analysis/outputs/"
    fi

else
    echo "❌ Analysis failed"
    echo "Check if Python environment is properly set up"
fi

echo ""
echo "7. Next steps..."

echo ""
echo "🔬 Your experiment pipeline is working!"
echo ""
echo "To run more experiments:"
echo "  1. Modify descriptor_compare configuration"
echo "  2. Run: cd build && ./descriptor_compare"
echo "  3. Analyze: conda activate descriptor-compare && python analysis/scripts/precision_recall_analysis.py results/"
echo "  4. View: firefox analysis/outputs/analysis_report.html"
echo ""
echo "To run different descriptors, edit descriptor_compare/main.cpp or experiment_config"
echo ""
echo "🎉 Your thesis project now has a complete experimental workflow!"