cmake_minimum_required(VERSION 3.27)
project(descriptor_compare)

set(CMAKE_CXX_STANDARD 17)

# Find OpenCV, Boost, and TBB packages in the system
find_package(OpenCV REQUIRED)
find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(TBB REQUIRED)
#find_package(Torch REQUIRED)



# Set compiler flags
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -march=native")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -march=native")

# Include directories for OpenCV, Boost, and TBB headers
#include_directories(${OpenCV_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS})
#include_directories(${OpenCV_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${Torch_INCLUDE_DIRS} /usr/include/oneapi/tbb /opt/libtorch)
include_directories(${OpenCV_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${Torch_INCLUDE_DIRS} /usr/include/oneapi/tbb)

add_executable(descriptor_compare
        descriptor_compare/main.cpp
        keypoints/VanillaSIFT.cpp
        keypoints/VanillaSIFT.h
        keypoints/DSPSIFT.cpp
        keypoints/DSPSIFT.h
        keypoints/HoWH.cpp
        keypoints/HoWH.h
        keypoints/HoNC.cpp
        keypoints/HoNC.h
        keypoints/RGBSIFT.cpp
        keypoints/RGBSIFT.h
        descriptor_compare/image_processor.cpp
        descriptor_compare/image_processor.hpp
        descriptor_compare/experiment_config.hpp
        descriptor_compare/processor_utils.cpp
        descriptor_compare/processor_utils.hpp
        descriptor_compare/experiment_config.cpp
        descriptor_compare/locked_in_keypoints.cpp
        descriptor_compare/locked_in_keypoints.hpp)

link_directories(${OpenCV_LIBRARY_DIRS})

# Link libraries to the executable
target_link_libraries(descriptor_compare
        ${OpenCV_LIBS}
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        tbb)
#${TBB_LIBRARIES})

# Define the path to the data directory relative to the CMakeLists.txt file's location
set(DATA_PATH "${CMAKE_CURRENT_SOURCE_DIR}/data/")

# Define the path to the results directory relative to the CMakeLists.txt file's location
set(RESULTS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/results/")

# Define a path to the directory containing the precomputed keypoints
set(KEYPOINTS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/reference_keypoints/")

# Pass the data directory path to the C++ code
add_definitions(-DDATA_PATH="${DATA_PATH}")

# Pass the results directory path to the C++ code
add_definitions(-DRESULTS_PATH="${RESULTS_PATH}")

# Pass the keypoints directory path to the C++ code
add_definitions(-DKEYPOINTS_PATH="${KEYPOINTS_PATH}")
