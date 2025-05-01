project(samples)

file(GLOB SOURCES "../gpa/*")


if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../gpa/third_party)
    set(OCV_SAMPLE_DATA "")
    set(MANDATORY_PATH true)
    set(MAKE_PATH_MADATORY @)
else()
    set(DEPLOY_DIR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../gpa/third_party/deploy")
    set(OpenCV_DIR ${DEPLOY_DIR_PATH}/lib/cmake/opencv4)

    set(OCV_SAMPLE_DATA ${CMAKE_CURRENT_SOURCE_DIR}/../gpa/third_party/opencv/samples/data)
    set(MANDATORY_PATH false)
    set(MAKE_PATH_MADATORY )

endif()
    find_package(OpenCV 4.8.0 REQUIRED)
    find_package(CUDA REQUIRED)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/internal/test.utils.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/internal/test.utils.hpp.in.config)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/internal/test.utils.hpp.in.config ${CMAKE_CURRENT_SOURCE_DIR}/sources/test.utils.hpp COPYONLY)



    #NEIF -> NPPI EXTRA IMAGE FILTERING, i.e. the lib compile in the first project.
    set(LIB_GPA_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/bin/include)
    set(LIB_GPA_LIB_DIR ${CMAKE_SOURCE_DIR}/bin/lib)
    set(LIB_GPA_LIB -lgpa)



    set(CMAKE_EXECUTABLE_SUFFIX ".exe")


    file(GLOB DEMO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sources/*.cpp")
    foreach(sample ${DEMO_FILES})
        get_filename_component(sample_name ${sample} NAME_WE)
        add_executable(${sample_name} ${sample})

        target_include_directories(${sample_name} PRIVATE ${OpenCV_INCLUDE_DIRS} ${LIB_GPA_INCLUDE_DIR} ${CUDA_INCLUDE_DIRS})
        target_link_directories(${sample_name} PRIVATE ${OpenCV_INCLUDE_DIRS}/../../lib  ${LIB_GPA_LIB_DIR} )
        target_link_libraries(${sample_name} PRIVATE ${OpenCV_LIBS} ${LIB_GPA_LIB})

        set_target_properties(${sample_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/app)

        install(FILES ${CMAKE_SOURCE_DIR}/bin/app/${sample_name}.exe DESTINATION app)


        add_custom_command(TARGET ${sample_name}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "${sample} was built successfully."
        )


    endforeach()

