cmake_minimum_required(VERSION 3.2.0)
project(prlib)

include_directories(src/*/* src/binarizations src/balance src/border_detection src/deblur src/denoise src/deskew
                    src/detectors src/thinning)

include_directories( ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/src)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

set(PRLIB_SRC_PATH {Projec})

# Options
option(BUILD_SAMPLES false)
option(BUILD_TESTS false)

file(GLOB_RECURSE HEADERS "src/*/*.h" "src/*.h")
file(GLOB_RECURSE SOURCES "src/*/*.cpp" "src/*.cpp" "src/*/*.h" "src/*.h")

foreach(_header ${HEADERS})
    file(COPY ${_header} DESTINATION ${CMAKE_BINARY_DIR}/include)
endforeach()

find_package(Leptonica)
find_package(OpenCV)

include_directories(${LEPTONICA_INCLUDE_DIR})
include_directories(${OpenCV_INCLUDE_DIRS})

message(STATUS "Build shared library: ${BUILD_SHARED_LIBS}")
if(${BUILD_SHARED_LIBS})
    add_library(prlib SHARED ${SOURCES})
else()
    add_library(prlib STATIC ${SOURCES})
endif()

target_link_libraries(prlib
        ${OpenCV_LIBS}
        ${LEPTONICA_LIBRARY})

# Building samples
message(STATUS "Build samples: ${BUILD_SAMPLES}")
if(${BUILD_SAMPLES})
    add_subdirectory(samples)
endif()

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/out_include)

install(FILES ${HEADERS}
        DESTINATION ${CMAKE_INSTALL_PREFIX}
        )


