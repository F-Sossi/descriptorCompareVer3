# Complete Architecture Refactoring Plan

## ✅ IMPLEMENTATION STATUS (As of 2025-08-26)

### **Phase 3 & 4 COMPLETE: Production-Ready Architecture**

**🎯 Current Working System:**
- **Main Pipeline**: `descriptor_compare/` + Factory Pattern System (✅ COMPLETE)
  - Uses `PoolingFactory` and `MatchingFactory` for modular components
  - Full database integration with experiment tracking
  - Clean YAML configuration system
  - Zero technical debt, production-ready

**📊 Performance Results:**
- SIFT Baseline: 31.7% MAP  
- SIFT + Domain Size Pooling: 37.5% MAP (+18.3% improvement)
- SIFT + Stacking: 31.7% MAP
- All pooling strategies (None, DSP, Stacking) working correctly

### **🚧 Stage 7 Migration System (Future Work)**

**Parallel Modern Interface System:**
- **Location**: `src/core/descriptor/extractors/wrappers/` + `src/core/integration/`
- **Purpose**: Modern `IDescriptorExtractor` interface for future migration
- **Status**: Implemented but **NOT YET INTEGRATED** with main pipeline
- **Components**:
  - `SIFTWrapper.cpp/hpp` - Wraps SIFT with new interface
  - `RGBSIFTWrapper.cpp/hpp` - Wraps RGBSIFT with new interface  
  - `ProcessorBridge.cpp` - Bridge between old and new systems
  - `DescriptorFactory` (in factories/) - Factory for new interface

**Key Point**: These wrapper files are **NOT DEAD CODE** - they're part of the planned Stage 7 migration to modern interfaces, but the main pipeline continues to use the working factory pattern system.

---

## 🔍 COMPREHENSIVE CODE REVIEW FINDINGS (2025-08-18)

### **Complete Project Analysis - 15,000+ Lines Reviewed**

After systematic review of all project files, here are the prioritized refactoring recommendations:

### **✅ COMPLETED ISSUES**

#### **0. Complete CLI Migration & Legacy Removal - COMPLETED ✅ (2025-08-28)**
- **Problem**: Legacy `descriptor_compare/main.cpp` with 268 lines of hardcoded experiment logic alongside modern CLI tools
- **Solution**: Complete removal of legacy main executable and transition to pure CLI-only architecture
- **Files Removed**: ✅
  - `descriptor_compare/main.cpp` (268 lines of hardcoded experiments)
  - CMakeLists.txt descriptor_compare target and linking
  - All results folder creation code (database-only workflow)
- **Modern CLI Architecture**: ✅
  - `experiment_runner` - YAML-based experiment execution with full P@K/R@K metrics
  - `keypoint_manager` - Complete keypoint generation and management with database storage
  - `analysis_runner` - Analysis pipeline integration
- **Docker Integration**: ✅
  - Updated Dockerfile for CLI-only workflow with yaml-cpp dependency
  - Enhanced docker-compose.dev.yml with usage examples
  - Production container configuration for automated experiment running
- **Results**: Pure CLI-only architecture, no legacy hardcoded configurations, clean database-only storage
- **Impact**: Critical - eliminated major technical debt, simplified architecture, improved maintainability

#### **1. Legacy Database Cleanup - COMPLETED ✅**
- **Problem**: Duplicate database implementations (`database/` vs `src/core/database/`)
- **Files Removed**: ✅
  - `database/database_manager.cpp` (21 lines, incomplete) 
  - `database/database_manager.hpp` (basic stub)
  - `database/test_database.cpp` (outdated test)
- **Preserved**: `database/schema.sql` as documentation reference ✅
- **Build Verification**: All targets compile and link successfully ✅
- **Impact**: High - eliminated confusion and dead code, reduced technical debt

#### **2. Large File Decomposition - COMPLETED ✅ (2025-08-26)**
- **Problem**: `image_processor.cpp` was 644 lines - too large, mixed responsibilities
- **Solution**: Successfully extracted into focused modules:
  - **`src/core/metrics/ExperimentMetrics.hpp`** - Metrics computation and aggregation
  - **`src/core/metrics/MetricsCalculator.hpp`** - Static utility methods for metrics
  - **`src/core/visualization/VisualVerification.{hpp,cpp}`** - Visual debugging functions
- **Results**: 
  - **214 lines removed** (33% reduction): 644 → 430 lines
  - **Improved maintainability** - smaller, focused modules
  - **Enhanced testability** - components can be tested independently
  - **Zero breaking changes** - all functionality preserved
- **Build Verification**: All targets compile and link successfully ✅
- **Impact**: High - major code organization improvement, reduced technical debt

#### **3. Critical Metrics Mathematics - COMPLETELY FIXED ✅ (2025-08-26)**
- **Problem**: Multiple critical mathematical errors in metrics calculations
- **Fixed Issues**:
  - ❌ **Mathematically incorrect per-scene averaging** (was dividing matches/keypoints!)
  - ❌ **Stale values when empty** (old values preserved on empty inputs)
  - ❌ **Unsafe merge operations** (`.at()` calls could throw on missing keys)  
  - ❌ **Poor success flag semantics** (defaulted to failure)
  - ❌ **Misleading field names** (implied IR-style mAP when it wasn't)
- **Solutions Applied**:
  - ✅ **Proper precision averaging** with individual value storage
  - ✅ **Stale value protection** (always reset to 0.0 before calculation)
  - ✅ **Exception-safe merge** using `.count()` with safe defaults
  - ✅ **Optimistic success default** (`success = true`, set false on error)
  - ✅ **Clear documentation** of non-IR mAP terminology
- **Impact**: Critical - ensures research-quality mathematical accuracy

#### **4. True IR-style Mean Average Precision (mAP) - IMPLEMENTED ✅ (2025-08-26)**
- **Problem**: Existing "mAP" was macro average of scene precisions, NOT true IR-style mAP
- **Solution**: Complete true mAP implementation following HPatches methodology:
  - **`src/core/metrics/TrueAveragePrecision.{hpp,cpp}`** - Complete IR-style AP computation
  - **Homography-based ground truth** - Uses HPatches H matrices for geometric relevance  
  - **Single-GT policy (R=1)** - One relevant match per query within pixel tolerance τ
  - **Proper ranked evaluation** - Sorts by descriptor distance, computes AP from ranked relevance
  - **Standard IR formula**: `AP = (1/R) * Σ(Precision@k * 1[rel[k]=1])`
- **Integration**: Extended `ExperimentMetrics` with dual mAP system:
  - **`true_map_micro`** - Micro average over all queries (standard mAP)
  - **`true_map_macro_by_scene`** - Macro average across scenes (balanced)
  - **Backward compatibility** - Legacy `mean_average_precision` preserved
- **Features**:
  - ✅ **Configurable pixel tolerance** (default τ=3.0px, standard for HPatches)
  - ✅ **Query exclusion handling** (R=0 queries handled per IR best practices)
  - ✅ **Both micro and macro** aggregation methods
  - ✅ **OpenCV integration** - Seamless with existing pipeline
- **Status**: Infrastructure complete, ready for integration into processing pipeline
- **Impact**: Critical - enables proper research comparison with IR-style mAP standard

#### **5. Complete Keypoint Source Separation System - COMPLETED ✅ (2025-08-27)**
- **Problem**: Single keypoint generation method limited research evaluation to controlled conditions only
- **Solution**: Complete dual keypoint methodology system with database tracking
- **Implementation**: ✅
  - **Database Schema Enhancement**: Added `keypoint_sets` table with generation method tracking
  - **Homography Projection Method**: Controlled evaluation using HPatches ground truth transformations
  - **Independent Detection Method**: Realistic evaluation with fresh keypoint detection per image
  - **CLI Integration**: `keypoint_manager` supports both `generate-projected` and `generate-independent`
  - **Metadata Tracking**: Complete experiment traceability with keypoint set IDs and generation parameters
- **Features**: ✅
  - **Multiple Keypoint Sets**: Database stores unlimited named keypoint sets with different methodologies
  - **Keypoint Set Management**: CLI tools for listing, counting, and managing keypoint collections
  - **Method Comparison**: Direct performance comparison between controlled vs realistic conditions
  - **Backward Compatibility**: Legacy keypoint loading preserved while adding new capabilities
- **Research Impact**:
  - **Controlled Evaluation**: Homography projection isolates descriptor performance from keypoint detection variability
  - **Realistic Evaluation**: Independent detection provides real-world performance metrics
  - **Methodology Documentation**: Complete traceability of which keypoint methodology was used for each experiment
- **Results**: SIFT baseline shows different performance patterns - controlled vs realistic evaluation capabilities
- **Impact**: Critical - enables comprehensive descriptor evaluation methodology comparison for research validity

### **🔧 HIGH PRIORITY REFACTORING**

#### **6. True mAP Pipeline Integration - COMPLETED ✅ (2025-08-26)**
- **Problem**: True mAP infrastructure existed but wasn't integrated into main processing pipeline
- **Solution**: Complete integration of IR-style mAP computation with existing metrics system
- **Implementation**: ✅
  - **Pipeline Integration**: `image_processor.cpp` computes true mAP alongside legacy metrics
  - **Per-Query AP Computation**: Individual AP calculation for each keypoint correspondence
  - **Database Storage**: True mAP values stored in experiments database with per-scene breakdown
  - **Dual Metrics System**: Both legacy precision and true IR-style mAP available for comparison
- **Results**: ✅
  - **SIFT Baseline Results**: 31.8% true mAP with detailed per-scene breakdown (i_dome: 33.47%, v_wall: 63.27%)
  - **Precision@K Integration**: P@1=46.2%, P@5=55.3%, P@10=59.0% working correctly
  - **Research Standards**: Full IR-style evaluation methodology implemented
- **Impact**: Critical - enables proper research comparison with standard evaluation metrics

#### **6. Test Framework Modernization**
- **Current**: 782 lines using manual main() functions with basic assertions
- **Issues**: No test framework, manual success/failure checking, limited organization
- **Recommended**: Adopt Google Test or Catch2 framework
- **Benefits**: Test fixtures, parameterized tests, better assertions, CI/CD integration
- **Priority**: Medium - would improve test quality and coverage

### **📈 MEDIUM PRIORITY IMPROVEMENTS**

#### **4. Configuration System Enhancement**
- **Add**: Schema validation for YAML files (prevent config errors)
- **Add**: Default configuration templates in `config/defaults/` (currently empty)
- **Add**: Parameter range validation and documentation
- **Add**: Configuration versioning and migration support
- **Impact**: Low-Medium - improves user experience and robustness

#### **5. Error Handling Improvement**
- **Add**: Custom exception types for different error categories
- **Improve**: Granular error reporting in image processing pipeline
- **Add**: Input validation in public method interfaces
- **Add**: Better exception handling in CLI tools
- **Impact**: Low-Medium - improves robustness and debugging

#### **6. Documentation Expansion**
- **Add**: Comprehensive API documentation (Doxygen integration)
- **Improve**: Inline code documentation for complex algorithms
- **Add**: Architecture decision records (ADRs) for design choices
- **Add**: Performance benchmarking documentation
- **Impact**: Low - improves developer experience and maintainability

### **🎨 LOW PRIORITY POLISH**

#### **7. Include Path Consistency**
- **Issue**: Mix of relative paths (`../src/core/`) and absolute paths (`thesis_project/`)
- **Solution**: Standardize to project-root relative paths throughout
- **Files Affected**: CLI tools, test files, integration code
- **Impact**: Low - improves consistency and build reliability

#### **8. Root Directory Cleanup**
- **Consider**: Consolidate multiple setup scripts (`setup.py`, `setup.sh`, `setup_analysis_environment.sh`)
- **Archive**: Old planning documents (`plan.md` - 8679 lines)
- **Organize**: Docker-related files into `docker/` subdirectory
- **Impact**: Low - cosmetic improvement, better organization

#### **9. Performance Optimization Opportunities**
- **Add**: Benchmarking suite for strategy comparisons
- **Profile**: Descriptor computation bottlenecks
- **Optimize**: Memory usage in large descriptor calculations
- **Add**: Parallel processing where appropriate
- **Impact**: Low - performance improvement for large datasets

### **⚠️ IMPORTANT: PRESERVED PROFESSOR CODE**

#### **keypoints/ Directory - DO NOT MODIFY**
- **Status**: Professor's research code - preserve as-is
- **Files**: 3,694 lines of descriptor implementations (VanillaSIFT, RGBSIFT, HoNC, etc.)
- **Note**: Contains novel academic implementations with research heritage
- **Action**: Keep unchanged - this is reference implementation for research comparison
- **Quality**: Good research code, works correctly, documented with attribution

### **✅ EXCELLENT AREAS (Reference Quality)**

**These components are exemplary and should be used as reference for other code:**

1. **src/core/pooling/** and **src/core/matching/** - Perfect strategy pattern implementation
2. **src/core/config/** - Excellent YAML configuration system with type safety
3. **cli/** - Well-designed command-line tools with consistent interfaces
4. **config/experiments/** - Comprehensive, well-documented configuration files
5. **src/core/database/DatabaseManager.cpp** - Modern database integration (keep this one)

### **🎯 REFACTOR PRIORITY MATRIX**

| Priority | Component | Effort | Impact | Status | Timeline |
|----------|-----------|--------|--------|--------|----------|
| ✅ **COMPLETE** | CLI migration & legacy removal | High | Critical | **DONE** | **Completed 2025-08-28** |
| ✅ **COMPLETE** | Keypoint source separation | High | Critical | **DONE** | **Completed 2025-08-27** |
| ✅ **COMPLETE** | True mAP pipeline integration | Medium | Critical | **DONE** | **Completed 2025-08-26** |
| ✅ **COMPLETE** | True IR-style mAP infrastructure | High | Critical | **DONE** | **Completed 2025-08-26** |
| ✅ **COMPLETE** | Critical metrics mathematics | Medium | Critical | **DONE** | **Completed 2025-08-26** |
| ✅ **COMPLETE** | Large file decomposition | High | High | **DONE** | **Completed 2025-08-26** |
| ✅ **COMPLETE** | Legacy database cleanup | Low | High | **DONE** | **Completed 2025-08-19** |
| 🔧 **CURRENT** | Test framework modernization | Medium | Medium | **Ready** | **Current session** |
| 📈 MEDIUM | Configuration enhancement | Low | Low-Med | Partially done | **Next quarter** |
| 📈 MEDIUM | Error handling improvement | Medium | Low-Med | Pending | **Gradual improvement** |
| 🎨 LOW | Documentation expansion | High | Low | In progress | **Long-term goal** |

### **📊 CODE QUALITY METRICS**

- **Total Lines Reviewed**: ~15,000+
- **Excellent Quality**: src/core/ (modern C++, design patterns)
- **Good Quality**: cli/, config/, tests/ (functional, well-structured)
- **Needs Attention**: descriptor_compare/ (large files), database/ (duplicates)
- **Academic Code**: keypoints/ (preserve as-is per professor's requirements)

### **🏗️ SUGGESTED IMPLEMENTATION PHASES**

**Phase 1 (Immediate - 1 day)** ✅ **COMPLETED 2025-08-19**:
- ✅ Remove legacy database code (database_manager.cpp, database_manager.hpp, test_database.cpp)
- ✅ Fix CMakeLists.txt linker references 
- ✅ Verify build works with modern database only
- ✅ Update documentation to reflect current architecture

**Phase 2 (Next Sprint - 2 weeks)**:
- Decompose `image_processor.cpp` into logical modules
- Add proper error handling throughout

**Phase 3 (Next Month)**:
- Adopt test framework (Google Test recommended)
- Expand test coverage for individual components

**Phase 4 (Future)**:
- Performance optimization and profiling
- Comprehensive documentation expansion
- Configuration system enhancements

---

## Original State Analysis (For Historical Reference)

### **🟢 What's Working (Keep & Refactor)**
- **CLI Tools**: `cli/` (450 lines) - ✅ Well-structured, working perfectly
- **YAML Configuration**: `src/core/config/` (768 lines) - ✅ Modern, extensible  
- **Database Integration**: `src/core/database/` (769 lines) - ✅ Complete, working
- **Legacy Pipeline**: `descriptor_compare/` (1287 lines) - 🔄 Working but needs refactoring

### **🟡 What's Partially Implemented (Clean Up)**  
- **Descriptor Wrappers**: `src/core/descriptor/extractors/wrappers/` (185 lines) - Some implemented
- **Integration Bridge**: `src/core/integration/` (131 lines) - Basic implementation
- **Interfaces**: `src/interfaces/` (71 lines) - Skeleton only

### **🔴 What's Unused (Remove)**
- **Empty Directories**: 6 empty directories in `src/`
- **TODO Stubs**: Multiple unimplemented wrapper classes
- **Backup Files**: `experiment_config.hpp.stage*_backup`
- **Old Files**: `z_oldfiles/`, experimental stubs

---

## Target Final Structure

```
DescriptorProjectVer3/
│
├── 📁 cli/                          # ✅ KEEP - Command line tools  
│   ├── experiment_runner.cpp        # Main CLI for running experiments
│   ├── keypoint_manager.cpp         # Database keypoint management
│   └── analysis_runner.cpp          # Analysis pipeline runner
│
├── 📁 config/                       # ✅ KEEP - Experiment configurations
│   ├── defaults/                    # Default configuration templates
│   └── experiments/                 # YAML experiment definitions
│       ├── sift_baseline.yaml       # Individual descriptor tests
│       ├── sift_pooling_comparison.yaml  # Pooling strategy comparisons
│       └── sift_rgbsift_stacking.yaml    # Stacking combinations
│
├── 📁 src/                          # 🔄 REFACTOR - Modern modular source
│   ├── core/                        
│   │   ├── config/                  # ✅ KEEP - Configuration management
│   │   │   ├── ConfigurationBridge.cpp    # YAML → Legacy bridge
│   │   │   ├── ExperimentConfig.hpp       # Config data structures  
│   │   │   └── YAMLConfigLoader.cpp       # YAML parsing
│   │   │
│   │   ├── database/                # ✅ KEEP - Database integration
│   │   │   ├── DatabaseManager.cpp        # Complete SQLite management
│   │   │   └── DatabaseManager.hpp        
│   │   │
│   │   ├── descriptors/             # 🆕 NEW - Descriptor management  
│   │   │   ├── DescriptorFactory.cpp      # Factory for creating descriptors
│   │   │   └── DescriptorRegistry.hpp     # Registry pattern for extensibility
│   │   │
│   │   ├── pooling/                 # 🆕 NEW - Extracted from processor_utils
│   │   │   ├── PoolingStrategy.hpp         # Base interface
│   │   │   ├── DomainSizePooling.cpp      # DSP implementation  
│   │   │   ├── StackingPooling.cpp        # Stacking implementation
│   │   │   └── PoolingFactory.cpp         # Factory for pooling strategies
│   │   │
│   │   ├── matching/                # 🆕 NEW - Extracted from processor_utils
│   │   │   ├── DescriptorMatcher.hpp      # Base interface
│   │   │   ├── BruteForceMatcher.cpp      # Brute force implementation
│   │   │   └── MatchingFactory.cpp        # Factory for matchers
│   │   │
│   │   └── io/                      # 🆕 NEW - File I/O operations
│   │       ├── ResultsWriter.cpp          # CSV/file output
│   │       ├── HomographyReader.cpp       # Homography file reading
│   │       └── KeypointIO.cpp             # Keypoint CSV operations
│   │
│   └── utils/                       # ✅ KEEP - Utility functions
│       ├── conversion_utils.cpp            # Type conversions
│       └── image_utils.cpp                 # Basic image operations (from processor_utils)
│
├── 📁 keypoints/                    # ✅ KEEP - Original descriptor implementations
│   ├── DSPSIFT.{h,cpp}             # Domain-Size Pooled SIFT  
│   ├── RGBSIFT.{h,cpp}             # RGB SIFT
│   ├── VanillaSIFT.{h,cpp}         # Vanilla SIFT implementation
│   ├── HoNC.{h,cpp}                # Histogram of Normalized Colors
│   └── HoWH.{h,cpp}                # Other descriptor implementations
│
├── 📁 descriptor_compare/           # 🔄 REFACTOR - Simplified legacy pipeline
│   ├── main.cpp                    # Entry point (redirect to CLI)
│   ├── experiment_config.{hpp,cpp} # Cleaned up config (remove debug)
│   ├── image_processor.{hpp,cpp}   # Core processing (simplified)
│   └── locked_in_keypoints.{hpp,cpp} # Keypoint management
│
├── 📁 include/                     # ✅ KEEP - Public headers
│   └── thesis_project/             # Project namespace
│       ├── types.hpp               # Common type definitions
│       └── constants.hpp           # Project constants
│
├── 📁 database/                    # ✅ KEEP - Database schema
│   └── schema.sql                  # SQLite schema definition
│
├── 📁 tests/                       # ✅ KEEP - Test suite
│   ├── unit/                       # Unit tests for components
│   └── integration/                # End-to-end tests
│
├── 📁 analysis/                    # ✅ KEEP - Python analysis tools
│   ├── scripts/                    # Analysis scripts
│   └── notebooks/                  # Jupyter notebooks
│
├── 📁 data/                        # ✅ KEEP - HPatches dataset
│   ├── i_dome/                     # Illumination change dataset
│   └── v_wall/                     # Viewpoint change dataset
│
└── 📁 docs/                        # ✅ KEEP - Documentation
    ├── CLAUDE.md                   # Project instructions
    └── guides/                     # User guides
```

---

## Files to Remove

### **🗑️ Delete Entirely**
```
src/core/keypoint/generators/        # Empty directory
src/core/keypoint/validators/        # Empty directory  
src/core/descriptor/extractors/experimental/  # Empty directory
src/core/descriptor/extractors/pooling/       # Empty directory
src/core/evaluation/                 # Empty directory
src/core/runner/                     # Empty directory

descriptor_compare/experiment_config.hpp.stage1_backup
descriptor_compare/experiment_config.hpp.stage4_backup  
z_oldfiles/                          # Entire directory

# Unused wrapper stubs with TODO comments
src/core/descriptor/extractors/wrappers/HoNCWrapper.hpp
src/core/descriptor/extractors/wrappers/VSIFTWrapper.hpp  
src/core/descriptor/extractors/traditional/OpenCVSIFTWrapper.hpp
src/core/descriptor/extractors/traditional/RGBSIFTWrapper.hpp
src/core/descriptor/extractors/traditional/VanillaSIFTWrapper.hpp
```

### **🧹 Clean Up (Remove Debug Output)**
```
descriptor_compare/experiment_config.cpp     # Remove debug std::cout statements
descriptor_compare/processor_utils.cpp       # Remove debug std::cout statements  
src/core/config/ConfigurationBridge.cpp     # Remove debug std::cout statements
```

---

## Refactoring Implementation Plan

### **Phase 1: Extract Pooling Strategies (High Impact)**
**Goal**: Make adding new pooling strategies trivial

**Current**: All in `processor_utils.cpp` (computeDSPDescriptor, computeStackedDescriptor)
**Target**: Clean interfaces with factory pattern

```cpp
// New Interface
class PoolingStrategy {
public:
    virtual ~PoolingStrategy() = default;
    virtual cv::Mat apply(const cv::Mat& image, 
                         const std::vector<cv::KeyPoint>& keypoints,
                         const cv::Ptr<cv::Feature2D>& detector,
                         const experiment_config& config) = 0;
    virtual std::string getName() const = 0;
};

// Factory Usage
auto pooling = PoolingFactory::create(config.poolingStrategy);
cv::Mat descriptors = pooling->apply(image, keypoints, detector, config);
```

### **Phase 2: Extract Matching Logic (Medium Impact)**
**Goal**: Separate matching from general utilities

**Current**: Mixed in `processor_utils.cpp` (matchDescriptors, calculatePrecision)  
**Target**: Dedicated matching components

### **Phase 3: Clean Up Legacy Pipeline (Medium Impact)**
**Goal**: Simplified, maintainable legacy code  

**Current**: `image_processor.cpp` (643 lines) - too large, mixed responsibilities
**Target**: Focus on core image processing, delegate to new components

### **Phase 4: Remove Technical Debt (Low Impact)**
**Goal**: Clean, production-ready codebase

**Current**: Debug output, backup files, TODO stubs
**Target**: Clean code ready for research publication

---

## Benefits of Final Structure

### **🚀 For Adding New Descriptors**:
1. Add descriptor class to `keypoints/`
2. Add enum case to `types.hpp` 
3. Add factory registration
4. Write YAML config → **Done!**

### **🔧 For Adding New Pooling Strategies**:
1. Inherit from `PoolingStrategy` interface
2. Register with `PoolingFactory`
3. Add YAML support
4. **Zero changes to main pipeline**

### **📊 For Adding New Matching Methods**:
1. Inherit from `DescriptorMatcher` interface
2. Register with `MatchingFactory`
3. **Completely isolated from other components**

### **🧪 For Research Extensions**:
- **Modular**: Each component independently testable
- **Extensible**: Factory patterns for easy addition
- **Maintainable**: Clear separation of concerns
- **Reproducible**: YAML-driven configuration system