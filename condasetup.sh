#!/bin/bash
# Conda Environment Setup for Stage 6 Analysis

echo "=== Setting Up Conda Environment for Analysis ==="
echo "================================================="

echo ""
echo "Let's find and activate your conda environment..."

echo ""
echo "1. Checking available conda environments..."

# Check if conda is available
if command -v conda &> /dev/null; then
    echo "✅ Conda found: $(conda --version)"

    echo ""
    echo "Available environments:"
    conda env list

    echo ""
    echo "Looking for your descriptor-compare environment..."

    # Check if the descriptor-compare environment exists
    if conda env list | grep -q "descriptor-compare"; then
        echo "✅ Found descriptor-compare environment!"
        echo ""
        echo "To activate it and install packages:"
        echo ""
        echo "conda activate descriptor-compare"
        echo "conda install pandas seaborn matplotlib numpy scikit-learn"
        echo ""
        echo "Or install everything from the environment.yml:"
        echo "conda env update -f environment.yml"

    else
        echo "❌ descriptor-compare environment not found"
        echo ""
        echo "Let's create it from your environment.yml:"
        if [ -f "environment.yml" ]; then
            echo "conda env create -f environment.yml"
        else
            echo "environment.yml not found - creating one..."

            cat > environment.yml << 'EOF'
name: descriptor-compare
channels:
  - conda-forge
  - defaults
dependencies:
  # Core development tools
  - cmake>=3.16
  - cxx-compiler
  - make
  - git

  # Core libraries for computer vision
  - opencv
  - boost-cpp

  # Threading library
  - tbb-devel

  # Python environment for analysis
  - python=3.11
  - numpy
  - pandas
  - matplotlib
  - seaborn
  - scikit-learn
  - jupyter

  # Additional analysis tools
  - scipy
  - plotly
  - pip
  - pip:
    - []
EOF

            echo "✅ Created environment.yml"
            echo "Now run: conda env create -f environment.yml"
        fi
    fi

else
    echo "❌ Conda not found"
    echo ""
    echo "Conda might not be in your PATH. Try:"
    echo "1. Check if conda is installed: ls ~/anaconda3/bin/conda || ls ~/miniconda3/bin/conda"
    echo "2. Initialize conda: ~/anaconda3/bin/conda init bash"
    echo "3. Restart your shell: exec bash"
    echo "4. Or source conda: source ~/anaconda3/etc/profile.d/conda.sh"
fi

echo ""
echo "=== Manual Steps to Set Up Conda Environment ==="
echo ""
echo "1. Activate conda (if needed):"
echo "   source ~/anaconda3/etc/profile.d/conda.sh"
echo "   # OR"
echo "   source ~/miniconda3/etc/profile.d/conda.sh"
echo ""
echo "2. Create/activate environment:"
echo "   conda env create -f environment.yml     # If creating new"
echo "   conda activate descriptor-compare       # Activate existing"
echo ""
echo "3. Install missing packages:"
echo "   conda install pandas seaborn matplotlib numpy"
echo ""
echo "4. Verify installation:"
echo "   python -c \"import pandas, seaborn; print('Success!')\""
echo ""
echo "5. Test Stage 6 analysis:"
echo "   cd build && make run_analysis"
EOF

chmod +x conda_setup_guide.sh

echo ""
echo "Let me also create a quick activation helper:"

cat > activate_conda_and_test.sh << 'EOF'
#!/bin/bash
# Quick Conda Activation and Test Script

echo "=== Activating Conda and Testing Analysis ==="
echo "=============================================="

# Try different common conda locations
CONDA_PATHS=(
    "$HOME/anaconda3/etc/profile.d/conda.sh"
    "$HOME/miniconda3/etc/profile.d/conda.sh"
    "/opt/anaconda3/etc/profile.d/conda.sh"
    "/opt/miniconda3/etc/profile.d/conda.sh"
)

echo "Looking for conda installation..."

CONDA_FOUND=false
for conda_path in "${CONDA_PATHS[@]}"; do
    if [ -f "$conda_path" ]; then
        echo "✅ Found conda at: $conda_path"
        echo "Sourcing conda..."
        source "$conda_path"
        CONDA_FOUND=true
        break
    fi
done

if [ "$CONDA_FOUND" = false ]; then
    echo "❌ Conda not found in common locations"
    echo "Try manually: source /path/to/your/conda/etc/profile.d/conda.sh"
    exit 1
fi

echo ""
echo "Available conda environments:"
conda env list

echo ""
echo "Attempting to activate descriptor-compare environment..."

if conda activate descriptor-compare 2>/dev/null; then
    echo "✅ Environment activated!"

    echo ""
    echo "Current Python: $(which python)"
    echo "Python version: $(python --version)"

    echo ""
    echo "Checking required packages..."

    # Check if packages are installed
    missing_packages=()
    for package in pandas numpy matplotlib seaborn; do
        if ! python -c "import $package" 2>/dev/null; then
            missing_packages+=($package)
        fi
    done

    if [ ${#missing_packages[@]} -eq 0 ]; then
        echo "✅ All packages are installed!"

        echo ""
        echo "Testing analysis scripts..."

        if [ -f "analysis/scripts/precision_recall_analysis.py" ]; then
            python analysis/scripts/precision_recall_analysis.py --help > /dev/null 2>&1
            if [ $? -eq 0 ]; then
                echo "✅ Analysis script loads successfully"
            else
                echo "⚠️ Analysis script has issues"
            fi
        fi

    else
        echo "⚠️ Missing packages: ${missing_packages[*]}"
        echo "Installing them now..."

        conda install -y ${missing_packages[*]}

        if [ $? -eq 0 ]; then
            echo "✅ Packages installed successfully"
        else
            echo "❌ Failed to install packages"
            exit 1
        fi
    fi

    echo ""
    echo "🎉 Conda environment is ready for Stage 6 analysis!"
    echo ""
    echo "You can now run:"
    echo "  cd build && make run_analysis"
    echo ""
    echo "To manually activate this environment in the future:"
    echo "  conda activate descriptor-compare"

else
    echo "❌ Failed to activate descriptor-compare environment"
    echo ""
    echo "Environment might not exist. Create it with:"
    echo "  conda env create -f environment.yml"
    echo ""
    echo "Or create it manually:"
    echo "  conda create -n descriptor-compare python=3.11 pandas numpy matplotlib seaborn scikit-learn"
    exit 1
fi
EOF

chmod +x activate_conda_and_test.sh