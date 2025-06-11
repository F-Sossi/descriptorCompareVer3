#!/bin/bash
# setup.sh - One-command setup for descriptor research project

set -e

echo "=== Descriptor Research Project Setup ==="

# Function to detect OS
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if [ -f /etc/arch-release ]; then
            echo "arch"
        elif [ -f /etc/debian_version ]; then
            echo "debian"
        else
            echo "linux"
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    else
        echo "unknown"
    fi
}

# Check if Docker is available
if command -v docker &> /dev/null && command -v docker-compose &> /dev/null; then
    echo "✅ Docker found. Using containerized environment (recommended for reproducibility)."

    echo "🐳 Building Docker environment..."
    docker build -t descriptor-research:dev --target development .

    echo "📦 Downloading HPatches dataset if not present..."
    if [ ! -d "data" ] || [ -z "$(ls -A data 2>/dev/null)" ]; then
        python3 setup.py
    fi

    echo "🚀 Docker environment ready!"
    echo ""
    echo "To start development:"
    echo "  docker run -it --rm -v \$(pwd):/workspace descriptor-research:dev"
    echo ""
    echo "Or with X11 forwarding (Linux):"
    echo "  docker run -it --rm -v \$(pwd):/workspace -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=\$DISPLAY descriptor-research:dev"

elif command -v mamba &> /dev/null || command -v conda &> /dev/null; then
    echo "🐍 Conda/Mamba found. Setting up conda environment..."

    # Use mamba if available (faster), otherwise conda
    CONDA_CMD="conda"
    if command -v mamba &> /dev/null; then
        CONDA_CMD="mamba"
    fi

    # Create environment
    $CONDA_CMD env create -f environment.yml

    echo "📦 Downloading HPatches dataset if not present..."
    if [ ! -d "data" ] || [ -z "$(ls -A data 2>/dev/null)" ]; then
        python3 setup.py
    fi

    echo "✅ Conda environment created!"
    echo ""
    echo "To activate and build:"
    echo "  conda activate descriptor-research"
    echo "  mkdir -p build && cd build"
    echo "  cmake .."
    echo "  make -j\$(nproc)"

else
    echo "🔧 Setting up native environment..."

    OS=$(detect_os)
    echo "Detected OS: $OS"

    case $OS in
        arch)
            echo "Installing dependencies with pacman..."
            sudo pacman -Syu --needed --noconfirm base-devel cmake git python python-pip \
                opencv boost tbb python-numpy python-matplotlib
            ;;
        debian)
            echo "Installing dependencies with apt..."
            sudo apt-get update
            sudo apt-get install -y build-essential cmake git python3 python3-pip \
                libopencv-dev libopencv-contrib-dev libboost-all-dev libtbb-dev \
                python3-numpy python3-matplotlib
            ;;
        macos)
            echo "Installing dependencies with brew..."
            if ! command -v brew &> /dev/null; then
                echo "Homebrew not found. Please install it first:"
                echo '/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"'
                exit 1
            fi
            brew install cmake opencv boost tbb python numpy matplotlib
            ;;
        *)
            echo "❌ Unsupported OS for native setup. Please use Docker or install dependencies manually."
            echo ""
            echo "Required dependencies:"
            echo "  - CMake (>=3.16)"
            echo "  - OpenCV (with contrib modules)"
            echo "  - Boost (system, filesystem)"
            echo "  - TBB"
            echo "  - Python 3 with numpy, matplotlib"
            exit 1
            ;;
    esac

    # Install Python packages
    pip3 install --user pybind11 torch torchvision conan

    # Download dataset
    echo "📦 Downloading HPatches dataset if not present..."
    if [ ! -d "data" ] || [ -z "$(ls -A data 2>/dev/null)" ]; then
        python3 setup.py
    fi

    echo "✅ Native setup complete!"
    echo ""
    echo "To build:"
    echo "  mkdir -p build && cd build"
    echo "  cmake .."
    echo "  make -j\$(nproc)"
fi

echo ""
echo "=== Setup Complete ==="
echo ""
echo "Next steps:"
echo "1. Choose your development environment (Docker/Conda/Native)"
echo "2. Build the project using the instructions above"
echo "3. Run experiments: ./build/descriptor_compare"
echo ""
echo "For CLion integration, import the project and configure multiple CMake profiles:"
echo "  - Native-Debug: -DCMAKE_BUILD_TYPE=Debug"
echo "  - Native-Release: -DCMAKE_BUILD_TYPE=Release"
echo "  - Python-Bridge: -DBUILD_PYTHON_BRIDGE=ON"
