# Multi-stage build for descriptor research
FROM ubuntu:22.04 AS base

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    python3 \
    python3-pip \
    pkg-config \
    wget \
    curl \
    # OpenCV dependencies
    libopencv-dev \
    libopencv-contrib-dev \
    opencv-data \
    python3-opencv \
    # Boost
    libboost-all-dev \
    # Threading
    libtbb-dev \
    # X11 for potential GUI operations
    libx11-dev \
    libxext-dev \
    libxrender-dev \
    libxtst-dev \
    # ==================== ADD SQLITE3 HERE ====================
    # SQLite3 development package
    libsqlite3-dev \
    sqlite3 \
    # ==========================================================
    # Additional utilities
    htop \
    vim \
    nano \
    && rm -rf /var/lib/apt/lists/*

# Install Python packages for research
# Fix: Quote the numpy version constraint
RUN pip3 install \
    "numpy<2" \
    matplotlib \
    pandas \
    seaborn \
    scikit-learn \
    jupyter \
    pybind11 \
    torch \
    torchvision \
    conan \
    --no-cache-dir

# Development stage - includes additional tools
FROM base AS development

# Install additional development tools
RUN apt-get update && apt-get install -y \
    gdb \
    valgrind \
    clang-format \
    clang-tidy \
    && rm -rf /var/lib/apt/lists/*

# Setup Conan
RUN conan profile detect --force

# Create working directory
WORKDIR /workspace

# Set environment for OpenCV
ENV PKG_CONFIG_PATH=/usr/lib/pkgconfig:/usr/lib/x86_64-linux-gnu/pkgconfig
ENV DISPLAY=:0

# Default command for development
CMD ["bash"]

# Production stage - minimal runtime
FROM base AS production

WORKDIR /workspace
CMD ["./build/descriptor_compare"]