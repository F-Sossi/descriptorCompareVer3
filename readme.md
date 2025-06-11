# Descriptor Research Project

A comprehensive computer vision framework for comparing and evaluating image descriptors (SIFT, RGBSIFT, HoNC, etc.) using domain size pooling, stacking, and other advanced processing techniques.

## Table of Contents
- [Quick Start](#quick-start)
- [Installation Methods](#installation-methods)
- [Development Environments](#development-environments)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Troubleshooting](#troubleshooting)

## Quick Start

### Option 1: Docker (Recommended - Works Everywhere)
```bash
git clone <your-repository-url>
cd descriptor-compare
python3 setup.py  # Downloads HPatches dataset
docker build -t descriptor-research:dev --target development .
docker-compose -f docker-compose.dev.yml up -d
docker-compose -f docker-compose.dev.yml exec descriptor-dev bash
cd /workspace/build-docker && cmake .. && make -j$(nproc)
./descriptor_compare
```

### Option 2: Native Linux (Manjaro/Arch)
```bash
git clone <your-repository-url>
cd descriptor-compare
sudo pacman -S base-devel cmake opencv boost tbb
python3 setup.py  # Downloads HPatches dataset
mkdir build && cd build && cmake .. && make -j$(nproc)
./descriptor_compare
```

### Option 3: Windows (Docker - Easiest)
```powershell
git clone <your-repository-url>
cd descriptor-compare
python setup.py  # Downloads HPatches dataset
docker build -t descriptor-research:dev --target development .
docker run -it --rm -v ${PWD}:/workspace descriptor-research:dev bash
# Inside container: cd /workspace/build-docker && cmake .. && make -j$(nproc) && ./descriptor_compare
```

## Installation Methods

### Docker Installation (Recommended for Reproducibility)

#### Prerequisites
```bash
# Install Docker and Docker Compose

# Linux (Manjaro/Arch)
sudo pacman -S docker docker-compose

# Linux (Ubuntu/Debian)
sudo apt install docker.io docker-compose

# macOS
brew install docker docker-compose
# OR install Docker Desktop from https://www.docker.com/products/docker-desktop/

# Windows
# Install Docker Desktop from https://www.docker.com/products/docker-desktop/
# Enable WSL2 integration if using WSL2

# Add user to docker group (Linux only)
sudo usermod -aG docker $USER
sudo systemctl start docker
sudo systemctl enable docker
# Log out and back in for group changes
```

#### Build and Run
```bash
# Clone repository
git clone <your-repository-url>
cd descriptor-compare

# Download dataset (required)
python3 setup.py

# Build Docker environment
docker build -t descriptor-research:dev --target development .

# Start development environment
docker-compose -f docker-compose.dev.yml up -d

# Enter development container
docker-compose -f docker-compose.dev.yml exec descriptor-dev bash

# Build project inside container
cd /workspace
mkdir build-docker && cd build-docker
cmake .. -DUSE_SYSTEM_PACKAGES=ON -DUSE_CONAN=OFF
make -j$(nproc)

# Run experiments
./descriptor_compare
```

#### Docker Environment Features
- ✅ **Reproducible builds** across all systems
- ✅ **Pre-configured OpenCV** with SIFT support
- ✅ **All dependencies included**
- ✅ **X11 forwarding** for GUI debugging
- ✅ **Volume mounting** for live code editing

### Native Installation

#### Manjaro/Arch Linux
```bash
# Install system dependencies
sudo pacman -S base-devel cmake git python python-pip
sudo pacman -S opencv boost tbb intel-tbb

# Install OpenCV contrib (for SIFT support)
yay -S opencv-contrib  # or paru -S opencv-contrib

# Clone and build
git clone <your-repository-url>
cd descriptor-compare
python3 setup.py  # Download HPatches dataset
mkdir build && cd build
cmake .. -DUSE_SYSTEM_PACKAGES=ON -DUSE_CONAN=OFF
make -j$(nproc)
./descriptor_compare
```

#### Ubuntu/Debian
```bash
# Install system dependencies
sudo apt update
sudo apt install build-essential cmake git python3 python3-pip
sudo apt install libopencv-dev libopencv-contrib-dev
sudo apt install libboost-all-dev libtbb-dev

# Clone and build
git clone <your-repository-url>
cd descriptor-compare
python3 setup.py  # Download HPatches dataset
mkdir build && cd build
cmake .. -DUSE_SYSTEM_PACKAGES=ON -DUSE_CONAN=OFF
make -j$(nproc)
./descriptor_compare
```

#### macOS
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake opencv boost tbb python3

# Clone and build
git clone <your-repository-url>
cd descriptor-compare
python3 setup.py  # Download HPatches dataset
mkdir build && cd build
cmake .. -DUSE_SYSTEM_PACKAGES=ON -DUSE_CONAN=OFF
make -j$(nproc)
./descriptor_compare
```

#### Windows

##### Option 1: Docker (Recommended)
```powershell
# Install Docker Desktop from https://www.docker.com/products/docker-desktop/

# Clone repository
git clone <your-repository-url>
cd descriptor-compare

# Download dataset
python setup.py

# Build and run in Docker
docker build -t descriptor-research:dev --target development .
docker run -it --rm -v ${PWD}:/workspace descriptor-research:dev bash

# Inside container:
cd /workspace
mkdir build-docker && cd build-docker
cmake .. -DUSE_SYSTEM_PACKAGES=ON -DUSE_CONAN=OFF
make -j$(nproc)
./descriptor_compare
```

##### Option 2: Native Windows (Advanced)
```powershell
# Install Visual Studio 2022 with C++ workload
# Install vcpkg package manager
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install

# Install dependencies via vcpkg
.\vcpkg install opencv[contrib]:x64-windows
.\vcpkg install boost:x64-windows
.\vcpkg install tbb:x64-windows

# Clone and build project
git clone <your-repository-url>
cd descriptor-compare
python setup.py  # Download HPatches dataset

# Build with Visual Studio
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake -DUSE_SYSTEM_PACKAGES=ON
cmake --build . --config Release
.\Release\descriptor_compare.exe
```

##### Option 3: WSL2 (Linux Subsystem)
```bash
# Install WSL2 and Ubuntu from Microsoft Store
# Follow Ubuntu installation instructions inside WSL2:

sudo apt update
sudo apt install build-essential cmake git python3 python3-pip
sudo apt install libopencv-dev libopencv-contrib-dev
sudo apt install libboost-all-dev libtbb-dev

git clone <your-repository-url>
cd descriptor-compare
python3 setup.py  # Download HPatches dataset
mkdir build && cd build
cmake .. -DUSE_SYSTEM_PACKAGES=ON -DUSE_CONAN=OFF
make -j$(nproc)
./descriptor_compare
```

## Development Environments

### CLion Integration (Recommended for Development)

#### Multi-Environment Setup
1. **Open project in CLion**
2. **Configure Docker Toolchain**:
   - Settings → Build → Toolchains
   - Add Docker toolchain: `Docker Ubuntu 22.04`
   - Image: `descriptor-research:dev`

3. **Create CMake Profiles**:
   - **Native-Debug**: Fast local development
   - **Docker-Debug**: Reproducible container development
   - **Native-Release**: Performance testing
   - **Docker-Release**: Reproducible performance testing

#### CLion CMake Profiles Configuration

**Native-Debug:**
```
Build type: Debug
CMake options: -DUSE_SYSTEM_PACKAGES=ON -DUSE_CONAN=OFF
Build directory: cmake-build-debug
```

**Docker-Debug:**
```
Build type: Debug
Toolchain: Docker Ubuntu 22.04
CMake options: -DUSE_SYSTEM_PACKAGES=ON -DUSE_CONAN=OFF
Build directory: cmake-build-docker-debug
```

#### Switching Environments
- **Select configuration** from CLion toolbar dropdown
- **Build/Debug/Run** seamlessly in either environment
- **Native**: Full speed, all debugging features
- **Docker**: Reproducible, shareable environment

### Command Line Development

#### Docker Development Workflow
```bash
# Start development environment
docker-compose -f docker-compose.dev.yml up -d

# Develop with live code mounting
docker-compose -f docker-compose.dev.yml exec descriptor-dev bash

# Build and test
cd /workspace/build-docker
make -j$(nproc)
./descriptor_compare

# Stop environment
docker-compose -f docker-compose.dev.yml down
```

#### Native Development Workflow
```bash
# Standard CMake workflow
cd build
make -j$(nproc)
./descriptor_compare

# Or rebuild from scratch
cd .. && rm -rf build && mkdir build && cd build
cmake .. && make -j$(nproc)
```

## Usage

### Running Experiments

#### Basic Usage
```bash
# Run with default configuration
./descriptor_compare

# The program will:
# 1. Load HPatches dataset from data/
# 2. Extract descriptors using configured methods
# 3. Evaluate precision using homography validation
# 4. Save results to results/ directory
```

#### Dataset Requirements
The program expects the HPatches dataset in the `data/` directory:
```
data/
├── i_ajuntament/
│   ├── 1.ppm
│   ├── 2.ppm
│   ├── ...
│   ├── H_1_2
│   ├── H_1_3
│   └── ...
└── (other scene folders)
```

**Download dataset automatically:**
```bash
python3 setup.py
```

#### Configuration Options

**Descriptor Types:**
- `DESCRIPTOR_SIFT`: Standard SIFT
- `DESCRIPTOR_vSIFT`: Vanilla SIFT implementation
- `DESCRIPTOR_RGBSIFT`: RGB color SIFT
- `DESCRIPTOR_HoNC`: Histogram of Normalized Colors

**Processing Options:**
- `NONE`: No additional processing
- `DOMAIN_SIZE_POOLING`: Multi-scale descriptor pooling
- `STACKING`: Descriptor combination

**Modify configuration in:**
- `descriptor_compare/main.cpp` - Main experiment loop
- `descriptor_compare/experiment_config.hpp` - Configuration options

### Expected Output

```
Processing folder: /path/to/data/i_ajuntament
Results folder: /path/to/results/experiment_name/i_ajuntament
Hostname: container_id (if running in Docker)
Processing locked folder: /path/to/data/v_wall
Results folder: /path/to/results/experiment_name/v_wall
...
```

**Results saved to:**
- `results/experiment_name/scene_name/results.csv`
- Contains precision measurements for each image pair

## Project Structure

```
descriptor-compare/
├── CMakeLists.txt              # Build configuration
├── Dockerfile                  # Docker environment
├── docker-compose.dev.yml     # Development container config
├── setup.py                    # Dataset download script
├── environment.yml             # Conda environment (optional)
├── conanfile.txt              # Conan dependencies (optional)
│
├── keypoints/                  # Descriptor implementations
│   ├── VanillaSIFT.h/cpp      # Base SIFT implementation
│   ├── DSPSIFT.h/cpp          # Domain Size Pooling SIFT
│   ├── RGBSIFT.h/cpp          # RGB Color SIFT
│   ├── HoNC.h/cpp             # Histogram of Normalized Colors
│   └── HoWH.h/cpp             # Hue-weighted descriptor
│
├── descriptor_compare/         # Main application
│   ├── main.cpp               # Main experiment runner
│   ├── experiment_config.hpp  # Configuration management
│   ├── image_processor.hpp    # Image processing pipeline
│   ├── processor_utils.hpp    # Utility functions
│   └── locked_in_keypoints.hpp # Keypoint management
│
├── data/                      # HPatches dataset (created by setup.py)
├── results/                   # Experiment results
└── reference_keypoints/       # Pre-computed keypoints
```

## Troubleshooting

### Common Build Issues

#### OpenCV SIFT Not Found
```bash
# Verify SIFT support
python3 -c "import cv2; print('SIFT available:', hasattr(cv2, 'SIFT_create'))"

# If false, install OpenCV contrib:
# Manjaro: yay -S opencv-contrib
# Ubuntu: sudo apt install libopencv-contrib-dev
```

#### CMake Cache Issues
```bash
# Clean build directory
rm -rf build/
mkdir build && cd build
cmake .. && make
```

#### Docker Permission Issues
```bash
# Ensure user is in docker group
sudo usermod -aG docker $USER
# Log out and back in

# Fix file permissions
sudo chown -R $USER:$USER .
```

### Docker-Specific Issues

#### Container Won't Start
```bash
# Check Docker service
sudo systemctl status docker

# Rebuild image
docker-compose -f docker-compose.dev.yml build --no-cache
```

#### X11 Forwarding Issues
```bash
# Allow X11 connections
xhost +local:docker

# Test X11 in container
docker-compose -f docker-compose.dev.yml run descriptor-dev python3 -c "
import cv2
import numpy as np
img = np.zeros((100,100,3), dtype=np.uint8)
cv2.imshow('Test', img)
cv2.waitKey(1000)
"
```

#### Build Path Conflicts
```bash
# Use separate build directories
mkdir build-native    # for native builds
mkdir build-docker    # for Docker builds
```

### Dataset Issues

#### Dataset Not Found
```bash
# Verify dataset downloaded
ls data/
# Should show scene folders like i_ajuntament, v_wall, etc.

# Re-download if needed
python3 setup.py
```

#### Permission Issues
```bash
# Fix data directory permissions
sudo chown -R $USER:$USER data/
```

### Windows-Specific Issues

#### Docker Desktop Requirements
```powershell
# Ensure WSL2 is enabled
wsl --install
# Restart computer after WSL2 installation

# Enable virtualization in BIOS if needed
# Docker Desktop → Settings → General → Use WSL2 based engine
```

#### Visual Studio Build Issues
```powershell
# Ensure correct Visual Studio components installed:
# - MSVC v143 compiler toolset
# - Windows 10/11 SDK
# - CMake tools for C++

# Set environment variables if using vcpkg
$env:VCPKG_ROOT = "C:\path\to\vcpkg"
$env:CMAKE_TOOLCHAIN_FILE = "$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"
```

#### Path Length Issues
```powershell
# Enable long path support (Windows 10+)
# Run as Administrator:
New-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem" -Name "LongPathsEnabled" -Value 1 -PropertyType DWORD -Force
```

#### PowerShell Execution Policy
```powershell
# Allow script execution
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

## Development Guidelines

### Adding New Descriptors
1. **Create descriptor class** inheriting from appropriate base
2. **Add to DescriptorType enum** in `experiment_config.hpp`
3. **Register in createDescriptorExtractor()** factory method
4. **Test with both native and Docker builds**

### Contributing
1. **Test changes** in both native and Docker environments
2. **Ensure reproducible results** across environments
3. **Update documentation** for new features
4. **Follow existing code structure** and naming conventions

### Performance Testing
- **Use Release builds** for performance measurements
- **Test in Docker** to ensure reproducible timing
- **Profile with tools** like gdb, valgrind (available in Docker)

## Support

For issues, feature requests, or questions:
1. **Check troubleshooting section** above
2. **Verify environment setup** (native vs Docker)
3. **Test with minimal example** first
4. **Report with environment details** (OS, Docker version, etc.)

---

**Note**: This project requires the HPatches dataset for meaningful experiments. The `setup.py` script will automatically download and organize the dataset in the correct structure.
