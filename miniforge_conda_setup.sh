#!/bin/bash
# Miniforge Conda Setup for Stage 6 Analysis

echo "=== Activating Miniforge Conda Environment ==="
echo "==============================================="

echo ""
echo "You have miniforge3 installed - that's great!"
echo "Location: /home/frank/miniforge3"

echo ""
echo "1. Sourcing conda from miniforge..."

# Source conda from miniforge location
if [ -f "/home/frank/miniforge3/etc/profile.d/conda.sh" ]; then
    source /home/frank/miniforge3/etc/profile.d/conda.sh
    echo "✅ Conda sourced successfully"
else
    echo "❌ Conda script not found at expected location"
    exit 1
fi

echo ""
echo "2. Available environments:"
conda env list

echo ""
echo "3. Activating descriptor-compare environment..."

if conda activate descriptor-compare; then
    echo "✅ Environment activated!"
    echo "Current environment: $CONDA_DEFAULT_ENV"

    echo ""
    echo "4. Checking Python and packages..."
    echo "Python location: $(which python)"
    echo "Python version: $(python --version)"

    echo ""
    echo "5. Checking required packages..."

    missing_packages=()
    for package in pandas numpy matplotlib seaborn scikit-learn; do
        if python -c "import $package" 2>/dev/null; then
            echo "✅ $package is installed"
        else
            echo "❌ $package is missing"
            missing_packages+=($package)
        fi
    done

    if [ ${#missing_packages[@]} -eq 0 ]; then
        echo ""
        echo "🎉 All packages are installed!"

        echo ""
        echo "6. Testing analysis scripts..."

        if [ -f "analysis/scripts/precision_recall_analysis.py" ]; then
            echo "Testing precision_recall_analysis.py..."
            if python analysis/scripts/precision_recall_analysis.py --help > /dev/null 2>&1; then
                echo "✅ Analysis script works"
            else
                echo "⚠️ Analysis script has issues"
            fi
        fi

        if [ -f "results/sample/test_experiment.csv" ]; then
            echo ""
            echo "7. Running test analysis..."

            mkdir -p analysis/outputs

            if python analysis/scripts/precision_recall_analysis.py results/sample --output analysis/outputs; then
                echo "✅ Test analysis completed successfully"

                if python analysis/scripts/generate_report.py results/sample --output analysis/outputs; then
                    echo "✅ Test report generated successfully"

                    if [ -f "analysis/outputs/analysis_report.html" ]; then
                        echo ""
                        echo "🎊 SUCCESS! Generated test report:"
                        echo "📄 analysis/outputs/analysis_report.html"
                        echo ""
                        echo "Open it in your browser:"
                        echo "firefox analysis/outputs/analysis_report.html"
                        echo "# OR"
                        echo "google-chrome analysis/outputs/analysis_report.html"
                    fi
                else
                    echo "⚠️ Report generation failed"
                fi
            else
                echo "⚠️ Test analysis failed"
            fi
        else
            echo "⚠️ No sample data found for testing"
        fi

    else
        echo ""
        echo "Installing missing packages: ${missing_packages[*]}"

        if conda install -y ${missing_packages[*]}; then
            echo "✅ Missing packages installed successfully"

            echo ""
            echo "Testing again..."
            for package in ${missing_packages[*]}; do
                if python -c "import $package" 2>/dev/null; then
                    echo "✅ $package now working"
                else
                    echo "❌ $package still not working"
                fi
            done
        else
            echo "❌ Failed to install packages with conda"
            echo "Trying with pip..."

            if pip install ${missing_packages[*]}; then
                echo "✅ Packages installed with pip"
            else
                echo "❌ Failed to install packages"
                exit 1
            fi
        fi
    fi

    echo ""
    echo "=== Environment Setup Complete ==="
    echo ""
    echo "✅ Your conda environment is ready for Stage 6!"
    echo ""
    echo "To use it in the future:"
    echo "  conda activate descriptor-compare"
    echo ""
    echo "To test Stage 6 analysis:"
    echo "  cd build && make run_analysis"
    echo ""
    echo "Current environment status:"
    echo "  Environment: descriptor-compare"
    echo "  Python: $(python --version)"
    echo "  Location: $(which python)"

else
    echo "❌ Failed to activate descriptor-compare environment"
    echo ""
    echo "Try creating it fresh:"
    echo "  conda env remove -n descriptor-compare"
    echo "  conda env create -f environment.yml"
    exit 1
fi