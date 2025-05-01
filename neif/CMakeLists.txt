project(neif)
find_package(CUDA REQUIRED)

# Collect your source files
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Create a shared library
add_library(neif SHARED ${SOURCE_FILES})

# Include directories
target_include_directories(neif PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_include_directories(neif PRIVATE ${CUDA_INCLUDE_DIRS} )

target_link_libraries(neif PRIVATE ${CUDA_LIBRARIES})
target_link_directories(neif PRIVATE ${CUDA_TOOLKIT_ROOT_DIR}/lib64)
target_link_libraries(neif PRIVATE -lnppc -lnppial -lnppicc -lnppidei -lnppif -lnppig -lnppim -lnppist -lnppisu -lnppitc -lnpps)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/nppi_extra_image_filtering.hpp ${CMAKE_SOURCE_DIR}/bin/include/nppi_extra_image_filtering.hpp COPYONLY)

set_target_properties(neif PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/lib OUTPUT_NAME nppi_extra_image_filtering)


#install(TARGETS neif LIBRARY DESTINATION lib)

install(TARGETS neif LIBRARY DESTINATION lib PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/nppi_extra_image_filtering.hpp DESTINATION include)


add_custom_command(TARGET neif
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "libnppi_extra_image_filtering.so was built successfully."
)

