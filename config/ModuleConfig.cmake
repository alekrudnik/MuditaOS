set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

if(NOT DEFINED PROJECT_LIB_DIRECTORY )
    set(PROJECT_LIB_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/" CACHE STRING "Output path for static libraries")
    message("Setting PROJECT_LIB_DIRECTORY to ${PROJECT_LIB_DIRECTORY}")
endif()
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_LIB_DIRECTORY})
if(NOT DEFINED PROJECT_BUILD_DIRECTORY )
    set(PROJECT_BUILD_DIRECTORY "${CMAKE_SOURCE_DIR}/build" CACHE STRING "Output directory for building" )
    message("Setting PROJECT_BUILD_DIRECTORY to ${PROJECT_BUILD_DIRECTORY}")
endif()
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BUILD_DIRECTORY})

# add linux target cmake if exists for module
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/targets/Target_Linux.cmake)
    if(${PROJECT_TARGET} STREQUAL "TARGET_Linux")
        include(${CMAKE_CURRENT_SOURCE_DIR}/targets/Target_Linux.cmake)
    endif()
endif()

# add unittests
if(DEFINED BUILD_UNIT_TESTS)
    add_subdirectory(tests)
endif()
