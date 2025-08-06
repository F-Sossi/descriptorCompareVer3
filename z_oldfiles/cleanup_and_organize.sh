#!/bin/bash
# Clean up build directories and organize project

echo "=== Project Cleanup and Organization ==="
echo "========================================"

echo ""
echo "1. Cleaning up build directories..."

# Remove all temporary build directories
rm -rf build-stage*-test 2>/dev/null
rm -rf build-test* 2>/dev/null
rm -rf cmake-build-* 2>/dev/null

echo "✅ Removed temporary build directories"

echo ""
echo "2. Cleaning up temporary files..."

# Remove backup files
rm -f *.backup 2>/dev/null
rm -f *.bak 2>/dev/null
rm -f *~ 2>/dev/null
rm -f CMakeLists.txt.backup 2>/dev/null
rm -f CMakeLists_*.txt 2>/dev/null

# Remove test scripts we created during fixes
rm -f fix_*.sh 2>/dev/null
rm -f test_interfaces.cpp 2>/dev/null
rm -f fix_cmake_stage7.txt 2>/dev/null
rm -f conda_setup_guide.sh 2>/dev/null
rm -f activate_conda_and_test.sh 2>/dev/null

echo "✅ Removed temporary files"

echo ""
echo "3. Organizing stage scripts..."

# Create a stages directory for all stage scripts
mkdir -p stages
mv stage*.sh stages/ 2>/dev/null
mv validate_stage*.sh stages/ 2>/dev/null

echo "✅ Moved stage scripts to stages/ directory"

echo ""
echo "4. Creating main build directory..."

# Clean and recreate main build directory
if [ -d "build" ]; then
    echo "Cleaning existing build directory..."
    rm -rf build
fi

mkdir -p build
echo "✅ Created clean build directory"

echo ""
echo "5. Creating project structure documentation..."

cat > PROJECT_STRUCTURE.md << 'EOF'
# Descriptor Comparison Project Structure

## Directory Layout

```
DescriptorProjectVer3/
│
├── 📁 src/                        # New modular source code
│   ├── interfaces/                # Interface definitions (Stage 3)
│   │   └── IDescriptorExtractor.hpp
│   └── core/                      # Core implementations
│       ├── descriptor/            # Descriptor subsystem (Stage 7)
│       │   ├── factories/         # Factory classes
│       │   │   └── DescriptorFactory.{hpp,cpp}
│       │   └── extractors/        # Descriptor implementations
│       │       └── wrappers/      # Wrapper classes
│       │           ├── SIFTWrapper.{hpp,cpp}
│       │           └── RGBSIFTWrapper.{hpp,cpp}
│       ├── database/              # Database subsystem (Stage 5)
│       │   └── DatabaseManager.{hpp,cpp}
│       └── integration/           # Integration layer (Stage 7)
│           └── ProcessorBridge.{hpp,cpp}
│
├── 📁 include/                    # Public headers
│   └── thesis_project/            # Project namespace headers (Stage 2)
│       ├── types.hpp              # Common type definitions
│       └── constants.hpp          # Project constants
│
├── 📁 keypoints/                  # Original descriptor implementations
│   ├── DSPSIFT.{h,cpp}           # Domain-Size Pooled SIFT
│   ├── RGBSIFT.{h,cpp}           # RGB SIFT
│   ├── VanillaSIFT.{h,cpp}      # Vanilla SIFT
│   ├── HoNC.{h,cpp}              # Histogram of Normalized Colors
│   └── HoWH.{h,cpp}              # Other descriptors
│
├── 📁 descriptor_compare/         # Original main application
│   ├── main.cpp                  # Entry point
│   ├── experiment_config.{hpp,cpp} # Configuration system
│   ├── processor_utils.{hpp,cpp} # Processing utilities
│   ├── image_processor.{hpp,cpp} # Image processing
│   └── locked_in_keypoints.{hpp,cpp} # Keypoint management
│
├── 📁 database/                   # Legacy database code
│   └── database_manager.{hpp,cpp}
│
├── 📁 config/                     # Configuration files (Stage 4)
│   ├── default_config.yaml       # Default configuration
│   └── experiments/              # Experiment configurations
│
├── 📁 analysis/                   # Analysis tools (Stage 6)
│   ├── scripts/                  # Python analysis scripts
│   │   ├── precision_recall_analysis.py
│   │   └── generate_report.py
│   └── outputs/                  # Analysis results
│
├── 📁 tests/                     # Test suite
│   └── unit/                     # Unit tests
│       ├── simple_interface_test.cpp
│       ├── test_yaml_config.cpp
│       ├── database/             # Database tests
│       └── integration/          # Integration tests
│           └── test_stage7_migration.cpp
│
├── 📁 cli/                       # Command-line tools
│   └── analysis_runner.cpp      # Analysis CLI
│
├── 📁 stages/                    # Stage setup scripts
│   ├── stage1.sh through stage7.sh
│   └── validate_stage*.sh
│
├── 📁 build/                     # Build output (git-ignored)
│
├── 📁 data/                      # Datasets (git-ignored)
│
├── 📁 results/                   # Experiment results (git-ignored)
│
├── 📁 docs/                      # Documentation
│
├── 📄 CMakeLists.txt            # Build configuration
├── 📄 environment.yml           # Conda environment
├── 📄 Dockerfile                # Docker configuration
├── 📄 .gitignore                # Git ignore rules
└── 📄 README.md                 # Project documentation
```

## Build Commands

### Standard Build
```bash
cd build
cmake .. -DUSE_SYSTEM_PACKAGES=ON -DBUILD_INTERFACE_MIGRATION=ON
make -j$(nproc)
```

### Run Tests
```bash
cd build
ctest --output-on-failure
# Or specific test groups:
make run_all_tests
make run_database_tests
make run_interface_tests
```

### Run Analysis
```bash
conda activate descriptor-compare
cd build
make run_analysis
```

## Key Components by Stage

- **Stage 1-2**: Foundation and type system (`include/thesis_project/`)
- **Stage 3**: Interfaces (`src/interfaces/`)
- **Stage 4**: Configuration system (`config/`)
- **Stage 5**: Database integration (`src/core/database/`)
- **Stage 6**: Analysis tools (`analysis/`)
- **Stage 7**: Interface migration (`src/core/descriptor/`, `src/core/integration/`)
EOF

echo "✅ Created PROJECT_STRUCTURE.md"

echo ""
echo "6. Setting up clean build..."

cd build
cmake .. -DUSE_SYSTEM_PACKAGES=ON -DBUILD_INTERFACE_MIGRATION=ON -DBUILD_DATABASE=ON -DBUILD_ANALYSIS=ON > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✅ CMake configuration successful"
else
    echo "⚠️ CMake had issues - run manually to see errors"
fi

cd ..

echo ""
echo "=== Cleanup Complete ==="
echo ""
echo "Your project is now organized!"
echo ""
echo "📁 Clean directory structure:"
echo "  • src/         - New modular code"
echo "  • keypoints/   - Original descriptors"
echo "  • descriptor_compare/ - Main application"
echo "  • stages/      - All stage scripts"
echo "  • build/       - Clean build directory"
echo ""
echo "📄 Documentation:"
echo "  • PROJECT_STRUCTURE.md - Complete file layout"
echo ""
echo "Next steps:"
echo "  cd build"
echo "  make -j$(nproc)"
echo "  ./descriptor_compare  # Run main program"
echo "  ctest                 # Run all tests"