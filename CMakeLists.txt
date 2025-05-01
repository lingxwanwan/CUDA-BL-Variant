cmake_minimum_required(VERSION 3.17)


project(cudaAtScaleLib LANGUAGES CXX)

set(CMAKE_INSTALL_PREFIX /usr/local/cudaAtScale)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Python3 COMPONENTS Interpreter)
if(NOT Python3_FOUND)
    message(FATAL_ERROR "Python3 is required to generate the header file!")
endif()

# Call the Python script to generate the header file
execute_process(
    COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/write_header.py" "${CMAKE_CURRENT_SOURCE_DIR}"
    RESULT_VARIABLE generation_result
    COMMAND_ERROR_IS_FATAL ANY
)


# Add the source directory
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cudaGPA)
message("Lib cudaGPA.so was properly configures")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/neif)
message("Lib libnppi_extra_image_filtering.so was properly configured!")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/gpa)
message("Lib libgpa.so was properly configured!")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/samples)
message("Demonstation files were properly configured!")


add_dependencies(gpa neif cudaGPABF)
add_dependencies(demo1 gpa)
add_dependencies(demo2 gpa)





