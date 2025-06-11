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
    # Boost
    libboost-all-dev \
    # Threading
    libtbb-dev \
    # X11 for potential GUI operations
    libx11-dev \
    libxext-dev \
    libxrender-dev \
    libxtst-dev \
    # Additional utilities
    htop \
    vim \
    && rm -rf /var/lib/apt/lists/*

# Install Python packages for research
RUN pip3 install \
    numpy \
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

# Production stage - minimal runtime
FROM base AS production

WORKDIR /workspace

# Copy built application (will be mounted or copied)
# COPY --from=build /workspace/build/descriptor_compare /usr/local/bin/

# Default command for development
CMD ["bash"]
