#!/bin/bash
# Simple CSV Flattening Fix

echo "=== Simple CSV Flattening for Analysis ==="
echo "=========================================="

# Activate conda
source /home/frank/miniforge3/etc/profile.d/conda.sh
conda activate descriptor-compare

echo "✅ Conda environment activated: $CONDA_DEFAULT_ENV"

echo ""
echo "1. Creating clean flattened results directory..."
rm -rf analysis/flattened_results
mkdir -p analysis/flattened_results

echo ""
echo "2. Copying CSV files with simple naming..."

# Simple approach - copy all results.csv files with unique names
counter=1
find results -name "results.csv" -type f | while read csv_file; do
    # Extract meaningful parts for naming
    dir_path=$(dirname "$csv_file")
    descriptor_part=$(echo "$dir_path" | cut -d'/' -f2)  # Get descriptor config
    dataset_part=$(echo "$dir_path" | cut -d'/' -f3)     # Get dataset name

    # Create simple filename
    new_name="${descriptor_part}__${dataset_part}.csv"

    # Copy file
    cp "$csv_file" "analysis/flattened_results/$new_name"
    echo "✅ Copied: $csv_file -> $new_name"
done

echo ""
echo "3. Checking results..."
csv_count=$(ls analysis/flattened_results/*.csv 2>/dev/null | wc -l)
echo "CSV files copied: $csv_count"

if [ $csv_count -gt 0 ]; then
    echo ""
    echo "Sample files:"
    ls -la analysis/flattened_results/ | head -10

    echo ""
    echo "4. Testing a sample CSV file..."
    sample_file=$(ls analysis/flattened_results/*.csv | head -1)
    if [ -f "$sample_file" ]; then
        echo "Sample file: $sample_file"
        echo "First few lines:"
        head -5 "$sample_file"
        echo ""
        echo "Number of lines: $(wc -l < "$sample_file")"
    fi

    echo ""
    echo "5. Running analysis..."
    if python analysis/scripts/precision_recall_analysis.py analysis/flattened_results --output analysis/outputs; then
        echo "✅ Analysis successful!"

        echo ""
        echo "6. Generating report..."
        if python analysis/scripts/generate_report.py analysis/flattened_results --output analysis/outputs; then
            echo "✅ Report generated!"

            echo ""
            echo "🎊 SUCCESS! Your analysis is complete!"
            echo ""
            echo "View your results:"
            echo "  firefox analysis/outputs/analysis_report.html"
            echo ""
            echo "Analysis summary:"
            if [ -f "analysis/outputs/analysis_summary.txt" ]; then
                head -10 analysis/outputs/analysis_summary.txt
            fi

        else
            echo "❌ Report generation failed"
        fi
    else
        echo "❌ Analysis failed"
    fi

else
    echo "❌ No CSV files were copied successfully"
    echo ""
    echo "Let's debug this..."
    echo "Looking for CSV files:"
    find results -name "results.csv" -type f | head -5
    echo ""
    echo "Results directory structure:"
    tree results/ | head -20
fi