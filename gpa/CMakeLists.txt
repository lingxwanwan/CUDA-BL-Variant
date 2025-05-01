project(gpa)


find_package(CUDA REQUIRED)

set(BUILD_THIRD_PARTIES ON)

if(BUILD_THIRD_PARTIES)

    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party OR NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/deploy)
        set(BUILD_LOCAL_OCV True)
    else()
        set(BUILD_LOCAL_OCV False)
    endif()


    if(BUILD_LOCAL_OCV)

        # Create the third party directory
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/third_party)


        # Check if git is present
        find_package(Git REQUIRED QUIET)

        if(NOT EXISTS  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/opencv)
            message("Downloading Of OpenCV's Legacy Modules.")

            # Download the OpenCV's Legacy modules.
            # From these modultes we are going to compile
            # opencv_core
            # opencv_imgproc
            # opencv_imgcodecs
            # opencv_highgui
            execute_process(
                COMMAND ${GIT_EXECUTABLE} clone -b 4.8.0 https://github.com/opencv/opencv
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/third_party"
                RESULT_VARIABLE download_ocv_legacy_result
                COMMAND_ERROR_IS_FATAL ANY
                OUTPUT_QUIET
                ERROR_QUIET
                COMMAND_ECHO NONE
            )
        message("OpenCV's Legacy Modules Were Successfuly Downloaded!\n")
        endif()

        if(NOT EXISTS  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/opencv_contrib)
            message("Downloading Of OpenCV's Contribution Modules.")

            # Download the OpenCV's Contribution modules.
            # From these modultes we are going to compile
            # opencv_quality
            # opencv_cudaarithm
            # opencv_cudafilter
            execute_process(
                COMMAND ${GIT_EXECUTABLE} clone -b 4.8.0 https://github.com/opencv/opencv_contrib
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/third_party"
                RESULT_VARIABLE download_ocv_contrib_result
                COMMAND_ERROR_IS_FATAL ANY
                OUTPUT_QUIET
                ERROR_QUIET
                COMMAND_ECHO NONE
            )
        message("OpenCV's Contribution Modules Were Successfuly Downloaded!\n")
        endif()

        # Create the folders needed for the compilation and deployment.

        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/bin)
            file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/bin)
        endif()

        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/deploy)
            file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/deploy)
        endif()

        file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/third_party/bin)
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/third_party/deploy)

        # Check if TBB is present.
        execute_process(
            COMMAND dpkg -s libtbb2-dev
            RESULT_VARIABLE check_for_tbb
            OUTPUT_QUIET
        )

        if (NOT check_for_tbb EQUAL 0)
            set(HAVE_TBB OFF)
        else()
            set(HAVE_TBB ON)
        endif()

        # Check the intrinsics

        execute_process(
            COMMAND bash -c "cat /proc/cpuinfo | grep -o -w sse2"
            RESULT_VARIABLE check_simd
            OUTPUT_STRIP_TRAILING_WHITESPACE
            OUTPUT_QUIET
            ERROR_QUIET
            COMMAND_ECHO NONE
        )

        if(NOT check_simd EQUAL 0)
            set(HAS_SSE2 ON)
            list(APPEND SIMD_DISPATCH_LST "SSE2")
        else()
            set(HAS_SSE2 OFF)
        endif()

        execute_process(
            COMMAND bash -c "cat /proc/cpuinfo | grep -o -w sse3"
            RESULT_VARIABLE check_simd
            OUTPUT_STRIP_TRAILING_WHITESPACE
            OUTPUT_QUIET
            ERROR_QUIET
            COMMAND_ECHO NONE
        )

        if(NOT check_simd EQUAL 0)
            set(HAS_SSE3 ON)
            list(APPEND SIMD_DISPATCH_LST "SSE3")
        else()
            set(HAS_SSE3 OFF)
        endif()


        execute_process(
            COMMAND bash -c "cat /proc/cpuinfo | grep -o -w ssse3"
            RESULT_VARIABLE check_simd
            OUTPUT_STRIP_TRAILING_WHITESPACE
            OUTPUT_QUIET
            ERROR_QUIET
            COMMAND_ECHO NONE
        )

        if(NOT check_simd EQUAL 0)
            set(HAS_SSSE3 ON)
            list(APPEND SIMD_DISPATCH_LST "SSSE3")
        else()
            set(HAS_SSSE3 OFF)
        endif()

        execute_process(
            COMMAND bash -c "cat /proc/cpuinfo | grep -o -w sse4_1"
            RESULT_VARIABLE check_simd
            OUTPUT_STRIP_TRAILING_WHITESPACE
            OUTPUT_QUIET
            ERROR_QUIET
            COMMAND_ECHO NONE
        )

        if(NOT check_simd EQUAL 0)
            set(HAS_SSE4_1 ON)
            list(APPEND SIMD_DISPATCH_LST "SSE4_1")
        else()
            set(HAS_SSE4_1 OFF)
        endif()

        execute_process(
            COMMAND bash -c "cat /proc/cpuinfo | grep -o -w sse4_2"
            RESULT_VARIABLE check_simd
            OUTPUT_STRIP_TRAILING_WHITESPACE
            OUTPUT_QUIET
            ERROR_QUIET
            COMMAND_ECHO NONE
        )

        if(NOT check_simd EQUAL 0)
            set(HAS_SSE4_2 ON)
            list(APPEND SIMD_DISPATCH_LST "SSE4_2")
        else()
            set(HAS_SSE4_2 OFF)
        endif()

        execute_process(
            COMMAND bash -c "cat /proc/cpuinfo | grep -o -w AVX"
            RESULT_VARIABLE check_simd
            OUTPUT_STRIP_TRAILING_WHITESPACE
            OUTPUT_QUIET
            ERROR_QUIET
            COMMAND_ECHO NONE
        )

        if(NOT check_simd EQUAL 0)
            set(HAS_AVX ON)
            list(APPEND SIMD_DISPATCH_LST "AVX")
        else()
            set(HAS_AVX OFF)
        endif()

        execute_process(
            COMMAND bash -c "cat /proc/cpuinfo | grep -o -w AVX2"
            RESULT_VARIABLE check_simd
            OUTPUT_STRIP_TRAILING_WHITESPACE
            OUTPUT_QUIET
            ERROR_QUIET
            COMMAND_ECHO NONE
        )

        if(NOT check_simd EQUAL 0)
            set(HAS_AVX2 ON)
            list(APPEND SIMD_DISPATCH_LST "AVX2")
        else()
            set(HAS_AVX2 OFF)
        endif()


        if(HAS_AVX OR HAS_AVX2)
            set(HAS_AVX_FAMILY ON)
        else()
            set(HAS_AVX_FAMILY OFF)
        endif()

        if(HAS_SSE2 OR HAS_SSE3 OR HAS_SSSE3 OR HAS_SSE4_1 OR HAS_SSE4_2)
            set(HAS_SSE_FAMILY ON)
        else()
            set(HAS_SSE_FAMILY OFF)
        endif()

        if(HAS_AVX_FAMILY OR HAS_SSE_FAMILY)
            set(HAS_INTEL_AMD_SIMD ON)
        else()
            set(HAS_INTEL_AMD_SIMD OFF)
        endif()

        if(HAS_INTEL_AMD_SIMD)
            if(HAS_AVX2)
                set(SIMD_BASELINE "AVX2")
            elseif(HAS_AVX)
                set(SIMD_BASELINE "AVX")
            elseif(HAS_SSE4_2)
                set(SIMD_BASELINE "SSE4_2")
            elseif(HAS_SSE4_1)
                set(SIMD_BASELINE "SSE4_1")
            elseif(HAS_SSE3)
                set(SIMD_BASELINE "SSE3")
            elseif(HAS_SSSE3)
                set(SIMD_BASELINE "SSSE3")
            else()
                set(SIMD_BASELINE "SSE2")
            endif()
            list(JOIN SIMD_DISPATCH_LST " " SIMD_DISPATCH)
        endif()

        # Find nvidia-smi executable
        find_program(NVIDIA_SMI_EXECUTABLE nvidia-smi)

        # Check if nvidia-smi was found
        if(NOT NVIDIA_SMI_EXECUTABLE)
            message(FATAL_ERROR "nvidia-smi not found!")
        endif()

        # Step 1) Execute nvidia-smi command to get the architectures
        execute_process(COMMAND nvidia-smi --query-gpu=compute_cap --format=csv OUTPUT_VARIABLE GPU_INFO)

        # Step 2) Extract GPU architectures from the output
        string(REGEX MATCHALL "[0-9]+\\.[0-9]+" GPU_ARCHITECTURES "${GPU_INFO}")

        # Step 3) Remove the dot in the middle of the architectures
        foreach(ARCHITECTURE IN LISTS GPU_ARCHITECTURES)
            list(APPEND LOCAL_CUDA_ARCHITECTURES ${ARCHITECTURE})
        endforeach()


        list(APPEND CMAKE_ARGS_LIST " -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/third_party/deploy")
        list(APPEND CMAKE_ARGS_LIST " -DOPENCV_CONFIG_FILE_INCLUDE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/third_party/bin")
        list(APPEND CMAKE_ARGS_LIST " -DCMAKE_BUILD_TYPE=Release")
        list(APPEND CMAKE_ARGS_LIST " -DOPENCV_EXTRA_MODULES_PATH=${CMAKE_CURRENT_SOURCE_DIR}/third_party/opencv_contrib/modules")

        # What third party will be build.
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_IPP_IW=ON")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_JASPER=ON")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_JPEG=ON")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_OPENEXR=ON")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_OPENJPEG=ON")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_PNG=ON")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_TIFF=ON")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_ZLIB=ON")

        # What third party will not be build.
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_PERF_TESTS=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_TESTS=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_DOCS=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_EXAMPLES=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_JAVA=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_PACKAGE=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_PROTOBUF=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_TBB=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_WEBP=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_WITH_DYNAMIC_IPP=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_ITT=OFF")
        # Library options
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_SHARED_LIBS=ON")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_USE_SYMLINKS=ON")

        # Modules required for the tests
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_core=ON")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_cudaarithm=ON")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_cudev=ON") #If my memory is good this module is required by cudaarithm.
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_cudafilters=ON")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_highgui=ON")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_imgcodecs=ON")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_imgproc=ON")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_ml=ON") # Requires by the quality module.
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_quality=ON")

        # Modules not required.
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_3d=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_alphamat=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_apps=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_aruco=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_barcode=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_bgsegm=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_bioinspired=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_calib=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_ccalib=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_cudabgsegm=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_cudacodec=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_cudafeatures2d=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_cudaimgproc=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_cudalegacy=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_cudaobjdetect=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_cudaoptflow=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_cudastereo=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_cudawarping=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_cvv=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_datasets=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_dnn=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_dnn_objdetect=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_dnn_superres=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_dpm=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_face=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_features2d=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_flann=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_freetype=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_fuzzy=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_gapi=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_hdf=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_hfs=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_img_hash=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_intensity_transform=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_java=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_java_bindings_generator=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_js=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_js_bindings_generator=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_line_descriptor=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_mcc=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_objc_bindings_generator=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_objdetect=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_optflow=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_phase_unwrapping=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_photo=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_plot=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_python_bindings_generator=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_python_tests=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_rapid=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_reg=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_rgbd=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_saliency=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_sfm=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_shape=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_stereo=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_stitching=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_structured_light=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_superres=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_surface_matching=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_text=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_tracking=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_ts=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_video=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_videoio=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_videostab=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_viz=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_wechat_qrcode=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_world=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_xfeatures2d=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_ximgproc=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_xobjdetect=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_xphoto=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DBUILD_opencv_xstereo=OFF")

        # Hardware acceleration options
        list(APPEND CMAKE_ARGS_LIST " -DCV_DISABLE_OPTIMIZATION=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DCV_ENABLE_OPTIMIZATION=ON")
        if(HAS_INTEL_AMD_SIMD)
            list(APPEND CMAKE_ARGS_LIST " -DCPU_BASELINE=${SIMD_BASELINE}")
            list(APPEND CMAKE_ARGS_LIST " -DCPU_DISPATCH=${SIMD_DISPATCH}")
        endif()

        list(APPEND CMAKE_ARGS_LIST " -DENABLE_CCACHE=ON")
        list(APPEND CMAKE_ARGS_LIST " -DENABLE_OMIT_FRAME_POINTER=ON")
        list(APPEND CMAKE_ARGS_LIST " -DENABLE_PIC=ON")
        list(APPEND CMAKE_ARGS_LIST " -DENABLE_SOLUTION_FOLDERS=ON")
        list(APPEND CMAKE_ARGS_LIST " -DCUDA_FAST_MATH=ON")

        list(APPEND CMAKE_ARGS_LIST " -DCV_TRACE=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DENABLE_BUILD_HARDENING=OFF")
        # Because of the large number of flags, but also for the limited usage purpose of this compilation, this option would be to complicate to manage.
        list(APPEND CMAKE_ARGS_LIST " -DENABLE_CONFIG_VERIFICATION=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DENABLE_COVERAGE=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DENABLE_FLAKE8=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DENABLE_GNU_STL_DEBUG=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DENABLE_IMPL_COLLECTION=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DENABLE_INSTRUMENTATION=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DENABLE_LTO=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DENABLE_NOISY_WARNINGS=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DENABLE_PRECOMPILED_HEADERS=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DENABLE_PROFILING=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DENABLE_PYLINT=OFF")


        list(APPEND CMAKE_ARGS_LIST " -DEXECUTABLE_OUTPUT_PATH=${CMAKE_CURRENT_SOURCE_DIR}/third_party/bin/bin")

        list(APPEND CMAKE_ARGS_LIST " -DGENERATE_ABI_DESCRIPTOR=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DINSTALL_CREATE_DISTRIB=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DINSTALL_C_EXAMPLES=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DINSTALL_PYTHON_EXAMPLES=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DINSTALL_TESTS=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DINSTALL_TO_MANGLED_PATHS=OFF")

        # What is ON
        list(APPEND CMAKE_ARGS_LIST " -DWITH_ARITH_DEC=ON")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_ARITH_ENC=ON")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_CUBLAS=ON")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_CUDA=ON")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_CUFFT=ON")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_EIGEN=ON") # OpenCV will try to find EIGEN, if it fails this flag will be set to false.
        list(APPEND CMAKE_ARGS_LIST " -DWITH_IMGCODEC_PFM=ON")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_IMGCODEC_PXM=ON")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_IPP=ON")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_JASPER=ON")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_JPEG=ON")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_GTK=ON")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_LAPACK=ON")# OpenCV will try to find LAPACK, if it fails this flag will be set to false.
        list(APPEND CMAKE_ARGS_LIST " -DWITH_OPENCL=ON")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_OPENEXR=ON")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_OPENGL=ON")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_OPENJPEG=ON")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_PNG=ON")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_TIFF=ON")
        # What is OFF
        list(APPEND CMAKE_ARGS_LIST " -DWITH_1394=OFF") # No need to support firewire
        list(APPEND CMAKE_ARGS_LIST " -DWITH_ADE=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_ARAVIS=OFF") # No nedd to support GiGE-Vision Genicam
        list(APPEND CMAKE_ARGS_LIST " -DWITH_CLP=OFF") # No need of coinor optimization algorithms.
        list(APPEND CMAKE_ARGS_LIST " -DWITH_CUDNN=OFF") # No need of CUDNN support, because we are not doing deep learning.
        list(APPEND CMAKE_ARGS_LIST " -DWITH_FFMPEG=OFF") # No need for video support.
        list(APPEND CMAKE_ARGS_LIST " -DWITH_FREETYPE=OFF") # No need for OCR support.
        list(APPEND CMAKE_ARGS_LIST " -DWITH_GDAL=OFF") # No need for geotiff support.
        list(APPEND CMAKE_ARGS_LIST " -DWITH_GDCM=OFF") # No nedd for DICOM support.
        list(APPEND CMAKE_ARGS_LIST " -DWITH_GPHOTO2=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_GSTREAMER=OFF") # No need for video support.
        list(APPEND CMAKE_ARGS_LIST " -DWITH_GTK_2_X=OFF") # This one could be, but we activated the WITH_GTK flag.
        list(APPEND CMAKE_ARGS_LIST " -DWITH_HALIDE=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_HPX=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_IMGCODEC_HDR=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_IMGCODEC_SUNRASTER=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_JULIA=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_LIBREALSENSE=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_MATLAB=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_MFX=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_NVCUVID=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_OAK=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_OBSENSOR=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_ONNX=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_OPENCLAMDBLAS=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_OPENCLAMDFFT=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_OPENCL_SVM=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_OPENNI=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_OPENNI2=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_OPENVINO=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_OPENVX=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_PLAIDML=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_PROTOBUF=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_PVAPI=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_QT=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_QUIRC=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_PTHREADS_PF=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_TESSERACT=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_TIMVX=OFF") # The can be change if a NPU is around.
        list(APPEND CMAKE_ARGS_LIST " -DWITH_UEYE=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_UNIFONT=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_V4L=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_VA=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_VA_INTEL=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_VTK=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_VULKAN=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_WAYLAND=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_WEBNN=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_WEBP=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_XIMEA=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_XINE=OFF")
        list(APPEND CMAKE_ARGS_LIST " -DWITH_ITT=OFF")

        # Special Cases
        if(HAVE_TBB)
            list(APPEND CMAKE_ARGS_LIST " -DWITH_TBB=ON")
            list(APPEND CMAKE_ARGS_LIST " -DWITH_OPENMP=OFF")
        else()
            list(APPEND CMAKE_ARGS_LIST " -DWITH_TBB=OFF")
            list(APPEND CMAKE_ARGS_LIST " -DWITH_OPENMP=ON")
        endif()

        # Set Cuda Architectures
        list(APPEND CMAKE_ARGS_LIST "-DCUDA_ARCH_BIN=${LOCAL_CUDA_ARCHITECTURES}")

        list(JOIN CMAKE_ARGS_LIST " " OCV_CMAKE_ARGS)

        message("Configuration Of OpenCV In Progress.")
        execute_process(
            COMMAND bash -c "cmake ../opencv ${OCV_CMAKE_ARGS}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/bin"
            RESULT_VARIABLE ocv_compilation_preparation_result
            COMMAND_ERROR_IS_FATAL ANY
            OUTPUT_QUIET
            ERROR_QUIET
            COMMAND_ECHO NONE
        )
        message("Configuration Of OpenCV Done!\n")

        cmake_host_system_information(RESULT total_num_cores QUERY NUMBER_OF_LOGICAL_CORES)
        math(EXPR cores_to_use "${total_num_cores} * 3 / 4")


        message("Compilation Of OpenCV In Progress.")
        execute_process(
            COMMAND make -j -l${cores_to_use}
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/bin"
            RESULT_VARIABLE ocv_compilation_result
            ERROR_FILE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/bin/compile_error_log.txt"
            COMMAND_ERROR_IS_FATAL ANY
            OUTPUT_QUIET
            ERROR_QUIET
            COMMAND_ECHO NONE
        )
        message("Compilation Of OpenCV Done!\n")

        message("Deployment Of OpenCV In Progress.")
        execute_process(
            COMMAND make install
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/bin"
            RESULT_VARIABLE ocv_deployment_result
            COMMAND_ERROR_IS_FATAL ANY
            OUTPUT_QUIET
            ERROR_QUIET
            COMMAND_ECHO NONE
        )

        execute_process(
            COMMAND bash -c "ln -s $PWD/opencv*/opencv2 $PWD/opencv2"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/deploy/include"
            RESULT_VARIABLE ocv_convinient_link_result
            COMMAND_ERROR_IS_FATAL ANY
            OUTPUT_QUIET
            ERROR_QUIET
            COMMAND_ECHO NONE
        )
        message("Deployment Of OpenCV Done!\n")


    endif()

    set(DEPLOY_DIR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/deploy")

    set(OpenCV_DIR ${DEPLOY_DIR_PATH}/lib/cmake/opencv4)


    set(OCV_SAMPLE_DATA ${CMAKE_CURRENT_SOURCE_DIR}/third_party/opencv/samples/data)
    set(DOES_THIRD_PARTY_EXISTS false)
    set(MAKE_PATH_MADATORY )


else()

    set(OCV_SAMPLE_DATA "")
    set(DOES_THIRD_PARTY_EXISTS true)
    set(MAKE_PATH_MADATORY @)

endif()

    find_package(OpenCV 4.8.0 REQUIRED)

    # Current folder
    add_library(gpa SHARED ${CMAKE_CURRENT_SOURCE_DIR}/sources/gpa.cpp)

    target_include_directories(gpa PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

    # OpenCV
    target_include_directories(gpa PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_directories(gpa PRIVATE ${OpenCV_INCLUDE_DIRS}/../../lib)
    target_link_libraries(gpa PRIVATE ${OpenCV_LIBS})


    # NEIF (NPPI EXTRA IMAGE FILTERING) + CUDA_GPABF
    target_include_directories(gpa PRIVATE ${CMAKE_SOURCE_DIR}/bin/include)
    target_link_directories(gpa PRIVATE ${CMAKE_SOURCE_DIR}/bin/lib)
    target_link_libraries(gpa PRIVATE -lnppi_extra_image_filtering -lcuda_gpabf)

    # CUDA
    target_include_directories(gpa PRIVATE ${CUDA_INCLUDE_DIRS} )


    set_target_properties(gpa PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/lib OUTPUT_NAME gpa)


    install(TARGETS gpa LIBRARY DESTINATION lib)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/gpa.hpp DESTINATION include)



    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/gpa.hpp ${CMAKE_SOURCE_DIR}/bin/include/gpa.hpp COPYONLY)



    add_custom_command(TARGET gpa
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "libgpa.so was built successfully."
    )

