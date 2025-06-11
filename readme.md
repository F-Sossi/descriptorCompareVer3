# Descriptor Comparison
This project is based on the HPatches dataset [1]. It compares the performance of different descriptors on the HPatches dataset. 
This project modifies the original project to allow the primary comparison of the mean average precision to be done in a C++ environment.
The purpose is to expand the comparison to include legacy descriptors and allow the comparison to be made using different detectors and descriptor processing that would allow descriptors using color and other features to be compared efficiently.

## Setup
This project uses the Cmake build system. To build the project, you will need to have Cmake installed.
See the [Cmake website](https://cmake.org/) for instructions on how to install Cmake.

### Dependencies
This project uses the following libraries:
- OpenCV -> installation instructions can be found [here](https://docs.opencv.org)
- Boost -> installation instructions can be found [here](https://www.boost.org/)
- Intel TBB -> installation instructions can be found [here](https://software.intel.com/content/www/us/en/develop/tools/oneapi/components/onetbb.html)
- Pytorch CPP Libtorch -> installation instructions can be found [here](https://pytorch.org/cppdocs/installing.html)

### Downloading the HPatches dataset
1. run the setup.py script to download the HPatches dataset. The script will download the dataset and extract it to the data folder.
2. instructions to run cmake and build the project are provided below.

## Building the project
To build the project, you will need to run the following commands in the project root directory:
```bash
mkdir build
cd build
cmake ..
make
```

<!-- This is a comment. added to get a commit -->


### References
<a name="refs"></a>

[1] *HPatches: A benchmark and evaluation of handcrafted and learned local descriptors*, Vassileios Balntas*, Karel Lenc*, Andrea Vedaldi and Krystian Mikolajczyk, CVPR 2017.
*Authors contributed equally.
