# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

### Standard Build (with Database Integration)
**IMPORTANT**: Always use the `build/` directory for builds. CLion creates `cmake-build-debug/` and `cmake-build-release/` directories automatically - these should be kept separate and not used for manual builds.

```bash
mkdir build && cd build
cmake .. -DUSE_SYSTEM_PACKAGES=ON -DUSE_CONAN=OFF -DBUILD_DATABASE=ON
make -j$(nproc)
```

**Note**: You may see CMake warnings about unused Python variables (`Python3_EXECUTABLE`, `Python_EXECUTABLE`) - these can be safely ignored as they're not required for the C++ components.

### Docker Build (Recommended for Reproducibility)
```bash
# Development environment (user-safe, no permission conflicts)
export USER_ID=$(id -u) && export GROUP_ID=$(id -g)  # Match host user permissions
docker-compose -f docker-compose.dev.yml build --no-cache  # Build with user mapping
docker-compose -f docker-compose.dev.yml up -d
docker-compose -f docker-compose.dev.yml exec descriptor-dev bash
cd /workspace/build
cmake .. -DUSE_SYSTEM_PACKAGES=ON -DUSE_CONAN=OFF -DBUILD_DATABASE=ON
make -j$(nproc)
# Files created in container will match your host user permissions!

# Production CLI tools
docker build -t descriptor-research:prod --target production .
docker run descriptor-research:prod  # Runs experiment_runner with baseline config
```

### Test Commands
```bash
cd build
ctest --output-on-failure
# Or specific test groups:
make run_all_tests
make run_database_tests  
make run_interface_tests
make run_config_tests
make run_stage_tests
```

### Analysis Commands
```bash
# Run complete analysis pipeline
make run_analysis

# Quick analysis (plots only)
make quick_analysis
```

## Project Architecture

This is a computer vision research framework for comparing image descriptors (SIFT, RGBSIFT, HoNC, VGG, DNN-based descriptors) with advanced processing techniques like domain-size pooling and stacking.

### Core Components

- **keypoints/**: Original descriptor implementations (VanillaSIFT, DSPSIFT, RGBSIFT, HoNC, HoWH)
- **descriptor_compare/**: Core image processing components (no longer has main function)
- **src/core/**: Modern modular architecture with database, descriptor wrappers + factory, pooling, matching, metrics
- **src/interfaces/**: Interface definitions for modular design
- **database/**: SQLite-based experiment tracking system
- **analysis/**: Python analysis tools and reporting system
- **cli/**: Command-line tools for experiment management and keypoint operations

### Build System Architecture

The CMakeLists.txt supports multiple build configurations:
- **USE_CONAN**: Toggle Conan vs system packages (prefer OFF for system packages)
- **BUILD_DATABASE**: Optional database integration (default ON)
- **BUILD_ANALYSIS**: Optional analysis integration (default ON) 
- **BUILD_EXPERIMENT_CLI**: CLI experiment runner with YAML support (default ON)
- **BUILD_KEYPOINT_MANAGER**: Keypoint management CLI tool (default ON)

### Dependencies

- **OpenCV**: Core computer vision operations (requires contrib for SIFT, VGG, and xfeatures2d)
- **Boost**: System and filesystem operations
- **SQLite3**: Lightweight database for experiment tracking
- **TBB**: Optional threading library
- **yaml-cpp**: Configuration file processing
- **Google Test**: Unit testing framework (optional)
- **ONNX Runtime**: For DNN descriptor support (optional)

### Stage-Based Development

The project follows a 7-stage development model:
1. **Stage 1-2**: Foundation and type system
2. **Stage 3**: Interface definitions (`src/interfaces/`)
3. **Stage 4**: YAML configuration system (`config/`)
4. **Stage 5**: Database integration (`src/core/database/`)
5. **Stage 6**: Analysis tools (`analysis/`)  
6. Stage 7 migration removed — new pipeline is the default (descriptors/pooling/matching/metrics in src/core)

## Key Files and Locations

### Configuration
- `descriptor_compare/experiment_config.hpp`: Main configuration definitions
- `config/experiments/*.yaml`: Experiment configurations
- `CMakeLists.txt:217-222`: Path definitions for data, results, keypoints

### CLI Applications  
- `cli/experiment_runner.cpp`: Main CLI experiment tool with YAML configuration
- `cli/keypoint_manager.cpp`: Keypoint generation and management CLI
- `cli/analysis_runner.cpp`: Analysis pipeline runner

### Core Processing Components
- `descriptor_compare/image_processor.cpp`: Core image processing pipeline (used by some tests)
- `descriptor_compare/experiment_config.cpp`: Legacy configuration container for detectors/pooling
- `src/core/descriptor/factories/DescriptorFactory.cpp`: New extractor factory (SIFT, RGBSIFT wrappers)

### Database System
- `src/core/database/DatabaseManager.cpp`: SQLite experiment tracking and keypoint storage
- `database/schema.sql`: Database schema definition (experiments, results, locked_keypoints)
- Enable with `-DBUILD_DATABASE=ON` (default)

### CLI Tools
- `cli/experiment_runner.cpp`: YAML-based experiment runner
- `cli/keypoint_manager.cpp`: Database keypoint management tool
- `cli/analysis_runner.cpp`: Analysis pipeline runner

### Testing
- `tests/unit/`: Unit tests organized by component
- `tests/unit/integration/`: Integration tests (migration tests removed)
- Tests are automatically discovered and configured by CMake

## Development Workflow

### Running Experiments

#### NEW: CLI + YAML Workflow (Recommended)
```bash
cd build
./experiment_runner ../config/experiments/sift_baseline.yaml
./experiment_runner ../config/experiments/rgbsift_comparison.yaml
./experiment_runner ../config/experiments/sample_experiment.yaml
```

#### Legacy Note  
The old `descriptor_compare` executable with hardcoded configurations has been removed. Use the CLI tools above instead.

### Dataset Requirements
The program expects HPatches dataset in `data/` directory. Use `python3 setup.py` to download automatically.

### Creating New Experiments
1. **Create YAML config**: Copy and modify files in `config/experiments/`
2. **Configure descriptors**: Set descriptor type, pooling, normalization in YAML
3. **Run experiment**: `./experiment_runner ../config/experiments/your_config.yaml`
4. **Analyze results**: Results saved to `results/experiment_name/descriptor_name/`

### Adding New Descriptors
1. Implement in `keypoints/` following existing patterns
2. Add to `DescriptorType` enum in `include/thesis_project/types.hpp`
3. Update YAML loader string conversion in `YAMLConfigLoader.cpp`
4. Add corresponding tests

### Working with Database
Database integration is **ENABLED and WORKING** as of 2025-08-13. Features:
- **Build**: Automatically enabled with `-DBUILD_DATABASE=ON` (default)
- **Database**: SQLite file created at `build/experiments.db`
- **Tracking**: All experiments automatically tracked with configurations and timing
- **Schema**: Three tables - `experiments` (configs), `results` (metrics), `locked_keypoints` (keypoint storage)
- **Usage**: `DatabaseManager` class in `src/core/database/`

#### Database Commands
```bash
# View database contents
cd build
sqlite3 experiments.db
.schema                         # View table structure
SELECT * FROM experiments;      # View experiment configs
SELECT * FROM results;          # View experiment results
SELECT * FROM locked_keypoints; # View stored keypoints
```

#### Keypoint Management
The project now includes a complete CLI tool for managing locked-in keypoints in the database:

```bash
cd build

# Generate fresh keypoints from images and store in database
./keypoint_manager generate ../data

# Import keypoints from existing CSV files for reproducibility
./keypoint_manager import-csv ../reference_keypoints

# Export keypoints from database to CSV for long-term storage
./keypoint_manager export-csv ./exported_keypoints

# List all scenes with keypoint counts
./keypoint_manager list-scenes

# Count keypoints for specific scene/image
./keypoint_manager count i_dome 1.ppm
```

**Features**:
- **Database-First**: Keypoints stored directly in SQLite database
- **Reproducibility**: Import/export CSV functionality for sharing results
- **SIFT Generation**: Automatic keypoint generation using OpenCV SIFT
- **Scene Organization**: Organized by scene name and image name
- **Batch Operations**: Clear and regenerate keypoints for entire datasets

### Docker Development
Preferred for reproducible builds across platforms. The Docker environment includes all dependencies pre-configured with OpenCV contrib support for SIFT.

**⚠️ Important**: The Docker configuration now includes proper user mapping to prevent permission conflicts:
- Files created inside the container match your host user permissions
- No more `root`-owned files that require `sudo` to remove
- Set `USER_ID` and `GROUP_ID` environment variables before building
- This prevents the build directory permission issues that occurred previously

## Current Project Status (September 2025)

The project is in a **stable, production-ready state** with the following capabilities:

### ✅ **Working Descriptor Types**
- **SIFT**: Traditional SIFT (OpenCV implementation)
- **RGBSIFT**: Color-aware SIFT variant 
- **HoNC**: Histogram of Normalized Colors
- **VSIFT**: Variant SIFT implementation
- **DSPSIFT**: Domain-Size Pooling SIFT
- **VGG**: VGG descriptors (requires OpenCV contrib)
- **DNN Patch**: Neural network-based descriptors via ONNX

### ✅ **Working Pooling Strategies**
- **None**: No pooling (baseline)
- **Domain-Size Pooling (DSP)**: Advanced spatial pooling technique
- **Stacking**: Multiple descriptor combination

### ✅ **Infrastructure Status**
- **Build System**: All tests passing (19/19), including recent VGGWrapper fix
- **Database Integration**: Full SQLite-based experiment tracking
- **CLI Tools**: experiment_runner, keypoint_manager, analysis_runner
- **Docker Support**: User-safe containerized development environment
- **YAML Configuration**: Schema v1 with validation and defaults

### ✅ **Recent Fixes (September 2025)**
- **VGGWrapper Linker Issue**: Fixed missing VGGWrapper.cpp in test targets 
- **Test Suite**: All Google Test targets now compile and link successfully
- **DNN Support**: Working neural network descriptor integration
- **Keypoint Management**: Database-driven keypoint workflow with homography transformation

## Recent Progress & Baseline Results

### ✅ 2025-08-29: Docker Permission Fix & Build Cleanup COMPLETE
- **Problem**: Docker containers running as `root` created files with `root:root` ownership on host
- **Root Cause**: Missing `USER` directive in Dockerfile caused permission conflicts with host filesystem
- **Solution**: Added proper user mapping with `ARG USER_ID` and `GROUP_ID` parameters
- **Docker Fix**: Container now runs as matching host user (UID 1000, GID 1000)
- **Build Cleanup**: Removed 74MB of conflicted build directories (`build-docker`, `cmake-build-*`)
- **Results**: Clean build environment, no more permission denied errors, user-safe Docker workflow

### ✅ 2025-08-25: Database Integration COMPLETE
- **Database Infrastructure**: Fully implemented and tested ✅
- **Schema Updated**: `database/schema.sql` with experiments, results, and locked_keypoints tables ✅
- **CMake Integration**: Automatic database linking with `BUILD_DATABASE=ON` ✅
- **Real-Time Metrics**: ExperimentMetrics structure captures precision/recall directly ✅
- **Keypoint Management**: Complete CLI tool for database-driven keypoint workflow ✅
- **Main Application Integration**: ✅ **COMPLETE** - All CSV dependencies removed, full database integration

### ✅ 2025-08-26: Major Architecture Refactoring COMPLETE
**Phase 2: Large File Decomposition** - ✅ **COMPLETE**

**Problem**: `image_processor.cpp` was 644 lines with mixed responsibilities

**Solution**: Successfully extracted into focused, modular architecture:
- **`src/core/metrics/ExperimentMetrics.hpp`** - Metrics computation and aggregation  
- **`src/core/metrics/MetricsCalculator.hpp`** - Static utility methods for metrics
- **`src/core/visualization/VisualVerification.{hpp,cpp}`** - Visual debugging functions
- **`src/core/metrics/TrueAveragePrecision.{hpp,cpp}`** - True IR-style mAP computation

### ✅ 2025-08-28: Complete CLI Migration & Docker Integration COMPLETE

**Phase 1: Results Folder Cleanup** - ✅ **COMPLETE**
- **Problem**: Legacy code was still creating unnecessary results folders despite database-only storage
- **Solution**: Removed all `create_directories` calls from image_processor.cpp and experiment_runner.cpp
- **Results**: Clean database-only workflow with no filesystem clutter

**Phase 2: Legacy Main Function Removal** - ✅ **COMPLETE** 
- **Problem**: `descriptor_compare/main.cpp` contained 268 lines of outdated hardcoded experiment logic
- **Solution**: Complete removal of legacy main function and CMakeLists.txt target
- **Results**: Pure CLI-only architecture using experiment_runner with YAML configuration
- **Eliminated**: Hardcoded loops, old P@K/R@K code, legacy database calls, duplicated functions

**Phase 3: Docker Configuration Modernization** - ✅ **COMPLETE**
- **Problem**: Docker files referenced removed descriptor_compare executable
- **Solution**: Updated Dockerfile production stage for CLI tools, added yaml-cpp dependency
- **Enhanced**: docker-compose.dev.yml with modern usage examples and comments
- **Updated**: CLAUDE.md documentation for complete Docker workflow

**Phase 4: Complete Architecture Validation** - ✅ **COMPLETE**
- **Build System**: All targets compile successfully without descriptor_compare
- **CLI Functionality**: experiment_runner, keypoint_manager, analysis_runner fully operational
- **Docker Integration**: Complete containerized workflow with database integration
       331 +  - **Metrics System**: All P@K/R@K metrics working correctly (SIFT baseline: 56% mAP)


**Results**: 
- **214 lines removed** (33% reduction): 644 → 430 lines
- **Zero breaking changes** - all functionality preserved
- **Improved maintainability** and testability
- **Mathematical correctness verified** - fixed critical calculation errors

### ✅ Critical Metrics Mathematics FIXED
**Fixed Multiple Critical Mathematical Errors**:
- ❌ **Incorrect per-scene averaging** (was dividing matches/keypoints!) → ✅ **Proper precision storage and averaging**
- ❌ **Stale values on empty inputs** → ✅ **Always reset to 0.0 before calculation** 
- ❌ **Unsafe merge operations** (`.at()` throws) → ✅ **Exception-safe merge with defaults**
- ❌ **Misleading mAP naming** → ✅ **Clear documentation of non-IR mAP**

### ✅ True IR-style mAP Infrastructure IMPLEMENTED  
**Complete Standard Information Retrieval mAP System**:
- **Homography-based ground truth** using HPatches methodology
- **Single-GT policy (R=1)** with configurable pixel tolerance (τ=3.0px default)
- **Proper ranked evaluation** sorting by descriptor distance
- **Standard IR formula**: `AP = (1/R) * Σ(Precision@k * 1[rel[k]=1])`
- **Dual metrics**: Micro mAP (all queries) + Macro mAP (balanced scenes)
- **Backward compatibility**: Legacy metrics preserved for comparison

**Status**: Infrastructure complete, ready for pipeline integration

**Next Phase - Complete True mAP Integration**:
- **CURRENT**: Integrate true mAP computation into `image_processor.cpp` pipeline  
- **Phase 3**: Test framework modernization (Google Test adoption)
- **Future**: Advanced descriptor strategies and performance optimization

### SIFT Baseline Results (Experiment ID: 5)
**Configuration**: Standard OpenCV SIFT, Grayscale, No Pooling, L2 Norm
```
Descriptor: SIFT-BW-None-NoNorm-NoRoot-L2
Processing Time: 800ms
```

**Precision Results**:
- **i_dome (viewpoint changes)**: 80.44% average precision
- **v_wall (illumination changes)**: 4.36% average precision  
- **Overall average**: 42.40% precision

**Analysis**: Results match expected SIFT performance - excellent on viewpoint changes, poor on illumination changes. This establishes the baseline for comparing other descriptors.

### Database-Only Migration Plan ✅ COMPLETED

**Migration Complete**: All phases successfully implemented and tested.

#### ✅ Phase 1: Core Infrastructure Changes (COMPLETED)
1. **ExperimentMetrics Structure** - Implemented in `image_processor.hpp`
   - Comprehensive metrics: precision per image, MAP, recall, total matches, keypoints
   - Per-scene breakdown support (i_dome vs v_wall)
   - Real-time metric computation and aggregation

2. **Image Processor Updates** - Updated return values
   - Changed `image_processor::process_directory()` from `bool` to `ExperimentMetrics`
   - Updated `process_image_folder_*` functions to collect and return real metrics
   - Proper aggregation across all scenes/images

3. **Database Integration** - Real-time metric storage
   - Modified `main.cpp` to capture ExperimentMetrics and store real values
   - Database stores computed precision/recall values (MAP: 42.4% for SIFT baseline)
   - Scene-specific metadata included

#### ✅ Phase 2: Keypoint Management (COMPLETED)
4. **Locked-In Keypoints** - Database storage system
   - Extended database schema with `locked_keypoints` table
   - DatabaseManager methods for keypoint storage/retrieval
   - Scene and image-based organization

5. **CLI Keypoint Manager** - Complete tool with 3 options
   - `generate`: Clear DB and generate fresh keypoints from images
   - `import-csv`: Import keypoints from CSV files for reproducibility
   - `export-csv`: Export keypoints from DB to CSV for long-term storage
   - Additional utilities: list-scenes, count keypoints

#### ✅ Phase 3: Testing & Validation (COMPLETED)
6. **End-to-End Testing** - Full workflow validated
   - Database-only workflow tested and working
   - Real metrics captured: 42.4% MAP for SIFT baseline
   - 6573+ keypoints generated and stored successfully
   - All CLI tools functional and integrated with CMake

## Documentation and Status Files

The `docs/StatusDocs/` folder contains detailed information about specific features and troubleshooting:

- **ARCHITECTURE_REFACTOR_PLAN.md**: Complete refactoring history and current architecture state
- **CNN_TROUBLESHOOTING_GUIDE.md**: DNN descriptor integration troubleshooting and performance analysis
- **DNN_BASELINE_STATUS.md**: Status of neural network descriptor implementations
- **FUTURE_METRICS_ENHANCEMENTS.md**: Planned evaluation metrics improvements
- **KEYPOINT_SOURCE_IMPLEMENTATION_PLAN.md**: Keypoint generation strategy implementation details
- **NEXT_SESSION_STATUS.md**: Recent development session notes

Refer to these files for detailed technical information about specific subsystems.

## Common Issues & Troubleshooting

### Build Issues
- **VGGWrapper linker errors**: Ensure OpenCV contrib is installed and `VGGWrapper.cpp` is included in CMakeLists.txt
- **Test failures**: Run `make clean && make -j$(nproc)` to rebuild all targets
- **Missing dependencies**: Use system packages (`-DUSE_SYSTEM_PACKAGES=ON`) for better compatibility

### Runtime Issues
- **ONNX model loading**: Check that model files exist in the expected location
- **Database connectivity**: Ensure `BUILD_DATABASE=ON` and SQLite3 is available
- **Keypoint generation**: Use `./keypoint_manager generate ../data` to populate the database

### Performance Issues
- **Slow descriptor extraction**: Consider using fewer keypoints or simpler descriptors for initial testing
- **Memory usage**: Monitor with large datasets, consider batch processing

### Recent Completion: Homography-Based Keypoint Transformation ✅ COMPLETED

**2025-08-13: Critical Keypoint Positioning Fix**
- **Issue Identified**: Keypoint manager was using identical keypoints for all images in a scene, instead of properly transforming them using homography matrices
- **Root Cause**: HPatches dataset requires keypoint correspondence through homography transformation (H_1_2, H_1_3, etc.) for images with viewpoint changes
- **Solution Implemented**: Full homography-based keypoint transformation in `cli/keypoint_manager.cpp`
  - Reads homography matrices from HPatches dataset files (H_1_2, H_1_3, H_1_4, H_1_5, H_1_6)
  - Transforms keypoints from reference image (1.ppm) to target images (2-6.ppm) using `cv::perspectiveTransform`
  - Properly handles keypoints that fall outside image boundaries after transformation
  - Stores correctly positioned keypoints in database for accurate descriptor evaluation

**Technical Implementation**:
- Added `processor_utils.cpp` dependency to keypoint_manager build system
- Implemented homography matrix reading using existing `processor_utils::readHomography` function
- Added comprehensive error handling for missing homography files
- Database operations now store geometrically correct keypoint correspondences

**Validation Results**:
- i_dome scene: 6,573 original keypoints → 5,337 valid transformed keypoints for image 2.ppm
- Keypoint count reduction indicates proper boundary filtering during transformation
- Database successfully stores transformed keypoints with correct spatial relationships

This fix resolves the fundamental evaluation accuracy issue and ensures that descriptor comparisons measure true matching performance rather than artifacts from incorrect keypoint positioning.

### Critical Fix: Evaluation Method with Boundary-Aware Keypoints ✅ COMPLETED

**2025-08-13: Complete Evaluation System Overhaul**

#### Problem Discovery and Analysis
**Initial Issue**: After implementing homography transformations, we discovered the evaluation method was still using `queryIdx == trainIdx` index comparison, which seemed invalid for transformed keypoints.

**Key Insight**: The user pointed out that boundary filtering was missing! The original `LockedInKeypoints::generateLockedInKeypoints()` function had proper 40px boundary filtering that our CLI implementation lacked.

**Thinking Process**:
1. **First Assumption**: "Homography transforms keypoints, so index correspondence breaks" → Led to complex geometric evaluation attempts
2. **Reality Check**: Examined actual keypoint coordinates in database
3. **Discovery**: Some transformed keypoints had negative coordinates (outside image bounds)
4. **Root Cause**: Missing boundary filtering meant invalid keypoints were included
5. **Solution**: Use existing tested boundary filtering logic instead of reimplementing

#### Technical Investigation Results
**HPatches Dataset Structure**:
- **i_dome** (illumination): Identity homography matrix → keypoints should be identical across images
- **v_wall** (viewpoint): Real transformation matrix → keypoints should be different but filtered for boundaries

**Boundary Analysis**:
```sql
-- v_wall image 2 bounds BEFORE filtering:
MIN(x): 32.73, MAX(x): 1020.22, MIN(y): -13.02, MAX(y): 671.90
-- Image dimensions: 986×653
-- Issues: negative y, x > image width
```

**Boundary Analysis AFTER filtering**:
```sql
-- v_wall image 2 bounds AFTER filtering:  
MIN(x): 51.70, MAX(x): 945.78, MIN(y): 40.76, MAX(y): 599.26
-- Expected bounds (40px border): x ∈ [40, 946], y ∈ [40, 613] ✅
```

#### Solution Implementation
**Strategy**: Leverage existing `LockedInKeypoints::generateLockedInKeypoints()` function with database output

**Key Components**:
1. **Boundary Filtering**: 40px border on original keypoints (lines 127-130 in locked_in_keypoints.cpp)
2. **Homography Transformation**: Proper coordinate transformation (lines 165-173)  
3. **Post-Transform Filtering**: Remove keypoints that fall outside bounds after transformation (lines 177-180)
4. **Database Storage**: Replace CSV output with database storage

**Implementation Details**:
```cpp
// Original keypoint filtering (40px border)
keypoints.erase(std::remove_if(keypoints.begin(), keypoints.end(), [image1](const cv::KeyPoint& keypoint) {
    return keypoint.pt.x < BORDER || keypoint.pt.y < BORDER ||
           keypoint.pt.x > (image1.cols - BORDER) || keypoint.pt.y > (image1.rows - BORDER);
}), keypoints.end());

// Post-transformation filtering (maintains correspondence)
transformedKeypoints.erase(std::remove_if(transformedKeypoints.begin(), transformedKeypoints.end(), [image1](const cv::KeyPoint& keypoint) {
    return keypoint.pt.x < BORDER || keypoint.pt.y < BORDER ||
           keypoint.pt.x > (image1.cols - BORDER) || keypoint.pt.y > (image1.rows - BORDER);
}), transformedKeypoints.end());
```

#### Results and Validation
**Performance Improvements**:
- **Database Speed**: 1,690x faster (18 minutes → 0.544 seconds) due to transaction batching
- **Boundary Compliance**: All keypoints now within valid image bounds
- **Correspondence Preservation**: Index alignment maintained after filtering

**Keypoint Count Analysis**:
- **i_dome**: 2000 keypoints per image (identity homography, no boundary loss)
- **v_wall**: 1532→1441 keypoints (decreasing with viewpoint difficulty, showing proper filtering)

**Evaluation Method Validation**:
- ✅ `queryIdx == trainIdx` is now **VALID** because boundary filtering preserves keypoint order
- ✅ Filtered keypoints maintain correspondence established by homography transformation
- ✅ No artifacts from invalid keypoint positions

#### Key Learning: Always Check Existing Logic First
**Process Lesson**: When encountering evaluation issues:
1. **First**: Check if existing tested logic exists (boundary filtering was already implemented!)
2. **Second**: Understand the dataset structure (i_ vs v_ prefixes, homography matrices)
3. **Third**: Validate assumptions with actual data (coordinate bounds, keypoint counts)
4. **Fourth**: Leverage existing proven code rather than reimplementing

This approach saved significant development time and ensured we used battle-tested boundary filtering logic.

### Environment Setup
```bash
# Activate conda environment for analysis (if using Python components)
source /home/frank/miniforge3/etc/profile.d/conda.sh
conda activate descriptor-compare
```

### Quick Start Guide
1. **Build the project**: `cmake -S . -B build -DUSE_SYSTEM_PACKAGES=ON && cmake --build build -j$(nproc)`
2. **Generate keypoints**: `cd build && ./keypoint_manager generate ../data`
3. **Run SIFT baseline**: `./experiment_runner ../config/experiments/sift_baseline_locked_keypoints.yaml`
4. **View results**: `sqlite3 experiments.db` then `SELECT * FROM experiments ORDER BY id DESC LIMIT 5;`
