project(cudaGPABF)

cmake_policy(SET CMP0104 NEW)

# First set and enable cuda.
find_package(CUDAToolkit REQUIRED)
find_package(CUDA REQUIRED)



set(CUDAToolkit_ROOT ${CUDA_TOOLKIT_ROOT_DIR})
set(CUDA_SDK_TOOLKIT_DIR ${CUDA_TOOLKIT_ROOT_DIR})


# Automatic finding of the architecture"

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
    string(REPLACE "." "" ARCHITECTURE_WITHOUT_DOT ${ARCHITECTURE})
    list(APPEND CMAKE_CUDA_ARCHITECTURES ${ARCHITECTURE_WITHOUT_DOT})
endforeach()


set(CMAKE_CUDA_COMPILER ${CUDA_NVCC_EXECUTABLE})

enable_language(CUDA)

set(CUDA_USE_STATIC_CUDA_RUNTIME OFF)
#set(CMAKE_CUDA_FLAGS "-m64 --compiler-options -fno-strict-aliasing -use_fast_math --ptxas-options=-v --expt-relaxed-constexpr -O3 --display-error-number --source-in-ptx --extended-lambda -v")



# Collect your source files
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sources/*")


# Create a shared library
add_library(cudaGPABF SHARED ${SOURCE_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/include/cuda_gpabf.hpp)

set_target_properties(cudaGPABF PROPERTIES CUDA_STANDARD 17)
set_target_properties(cudaGPABF PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
set(CMAKE_CUDA_FLAGS "-m64 --expt-relaxed-constexpr -O3 --shared -Xcompiler -fPIC")

target_include_directories(cudaGPABF PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(cudaGPABF PRIVATE ${CUDA_INCLUDE_DIRS} )


target_link_directories(cudaGPABF PRIVATE ${CUDA_TOOLKIT_ROOT_DIR}/lib64)
target_link_libraries(cudaGPABF PRIVATE -lcudart)


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/cuda_gpabf.hpp ${CMAKE_SOURCE_DIR}/bin/include/cuda_gpabf.hpp COPYONLY)

set_target_properties(cudaGPABF PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/lib OUTPUT_NAME cuda_gpabf)


install(TARGETS cudaGPABF LIBRARY DESTINATION lib)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/cuda_gpabf.hpp DESTINATION include)

add_custom_command(TARGET cudaGPABF
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "libcuda_gpabf.so was built successfully."
)

