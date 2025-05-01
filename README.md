# CUDA At Scale For Enterprise Final Project V1
This project is my first endeavor developed for the final evaluation of Coursera's course "CUDA At Scale For Enterprise."

## Project Description

The focal point of this project revolves around the work of `Chaudhury et al.`: [Fast and Provably Accurate Bilateral Filtering](https://paperswithcode.com/paper/fast-and-provably-accurate-bilateral/review/). The authors' original MATLAB implementation can be found [here](https://www.mathworks.com/matlabcentral/fileexchange/56158-fast-and-accurate-bilateral-filtering).

My contributions include the implementation of four distinct versions of this algorithm. To meet the criteria for NPPI and CUDA kernel utilization, I developed two separate versions, each tailored to NPPI and CUDA. Additionally, I created two versions using OpenCV—one for CPU which can also benefit from the TAPI and the other leveraging OpenCV's CUDA modules. The TAPI (Transparent API) integrates OpenCL, allowing the two functions I implemented to offer three interfaces: CPU-only, GPU using OpenCL, and CUDA.

The OpenCV-based versions are designed to compare results against a widely-used, industry-acclaimed library. I meticulously extended each code version to function across 1 to 4 channels and various data types, including unsigned char, unsigned short, short, and single-precision floating-point. These types are the only ones supported for convolution in NPPI, OpenCV, and CUDA.

The NPPI implementation posed particular challenges, primarily due to Gaussian kernels computing variance for kernel sizes below 17 x 17. I embarked on a reverse engineering journey to deduce the variance computation, which was time-consuming. The lack of clarity of the documentation and irregular function patterns (e.g. the position of the scratch buffer, in Min - Max functions is not at the same position as for the Mean - Std function) in NPPI added unexpected complexity. Despite this, exploring NPPI was an engaging endeavor.

The CUDA implementation proved relatively straightforward. My main challenge was to design a vanilla separable filtering class. This choice allowed a single-time allocation of the two required buffers, resulting in performance improvements compared to a function-based approach that would have necessitated buffer allocation at each call. Future versions could explore further memory management enhancements, like utilizing texture memory to minimize global memory transfers.



## Code Organization

- `cudaGPA/`: Contains all CUDA implementation code. Compilation generates a dynamic library. The compilation of this project will generate a library file named `libcuda_gpabf.so`  and a header file `cuda_gpabf.hpp`, which will be placed in `bin/lib` and `bin/include` respectively.
- `neif/`: Holds NPPI implementation code. "neif" stand (Nppi Extra Image Filtering) generates a dynamic library. The compilation of this project will generate a library file named `libnppi_extra_image_filtering.so`  and a header file `nppi_extra_image_filtering.hpp`, which will be placed in `bin/lib` and `bin/include` respectively.
- `samples_gpa_lib/`: Backend for demo files. Provides a unified interface for both NPPI and CUDA implementations with OpenCV containers. Additionally, it contains OpenCV's and OpenCV CUDA's implementations, along with helper functions for the demonstration executables. The compilation of this project will generate a library file named `libgpa.so`  and a header file `gpa.hpp`, which will be placed in `bin/lib` and `bin/include` respectively.
- `samples/`: Creates two executables, `demo1.exe` and `demo2.exe`. `demo1.exe` applies the Gaussian-Polynomial Approximate Bilateral Filter (GPA-BF) to a single image. It compares the five GPA-BF implementations (OpenCV CPU, OpenCV-OpenCL, OpenCV-CUDA, NPPI, and custom CUDA) using Mean Square Error and displays execution times. `demo2.exe` applies each GPA-BF implementation to a set of images. The executables are created in `bin/app`.
- `bin/`: Created after compilation of all other projects. Contains a `header` folder and a `library` folder with header and library files from `cudaGPA`, `neif`, and `samples_gpa_lib`. Also includes `demo1` and `demo2` files.

### Arguments for `demo1`:
- `-h`, `-help`, `-usage`, or `-?`: Print help.
- `-path`, `-folder`,`-fd`,`-input_path`, `-input_folder`, or `-ifd` : (optional, default: empty string) Folder containing the file to open.
- `-filename`, `-f`, `-input_filename`, or `-if`: (optional or required) Filename of the file to open.
- `-output_path`, `-output_folder`, or `-ofd` : (optional, default: empty string) Folder where to write the results.
- `-flag`: (optional, default: gaussian) Filter kernel to use.
- `-sigma_range` or `-sr`: (optional, default: 40.0) Range kernel standard deviation.
- `-sigma_spatial` or `-sp`: (optional, default: 4.0) Spatial kernel standard deviation.
- `-box_width` or `-bw`: (optional, default: 40) Box filter width.
- `-epsilon` or `-eps`: (optional, default: 1e-3) Desired accuracy.

**Note**:
- If a compiled OpenCV version exists in the `samples_gpa_lib/third_party` folder, the `-input_filename` argument is optional, and an image will be randomly selected from `samples_gpa_lib/third_party/opencv/samples/data/`.
- Providing an output directory will deactivate the display mode.

### Arguments for `demo2`:
- `-h`, `-help`, `-usage`, or `-?`: Print help.
- `-path`, `-folder`,`-fd`,`-input_path`, `-input_folder`, or `-ifd` : (required or optional, if optional default: empty string) Folder containing the file to open.
- `-output_path`, `-output_folder`, or `-ofd` : (optional, default: empty string) Folder where to write the results.
- `-N`: (optional, default: 10) Number of images to process.
- `-flag`: (optional, default: gaussian) Filter kernel to use.
- `-sigma_range` or `-sr`: (optional, default: 40.0) Range kernel standard deviation.
- `-sigma_spatial` or `-sp`: (optional, default: 4.0) Spatial kernel standard deviation.
- `-box_width` or `-bw`: (optional, default: 40) Box filter width.
- `-epsilon` or `-eps`: (optional, default: 1e-3) Desired accuracy.

**Note**:
- If a compiled OpenCV version exists in the `samples_gpa_lib/third_party` folder, the `-input_folder` argument is optional, and N images will be randomly selected from `samples_gpa_lib/third_party/opencv/samples/data/`.
- Providing an output directory will deactivate the display mode.

