cmake_minimum_required(VERSION 3.2.0)
project(prlib)

include_directories(src src/*/* src/binarizations src/balance src/border_detection src/deblur src/denoise src/deskew
                    src/detectors src/thinning)

set(CMAKE_CXX_STANDARD 11)

# Options
option(BUILD_SAMPLES false)
option(BUILD_TESTS false)

file(GLOB_RECURSE HEADERS "src/*/*.h" "src/*.h")
file(GLOB_RECURSE SOURCES "src/*/*.cpp" "src/*.cpp" "src/*/*.h" "src/*.h")

#find_package(Leptonica)
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
	#${LEPTONICA_LIBRARY}
	-llept)

# Building samples
message(STATUS "Build samples: ${BUILD_SAMPLES}")
if(${BUILD_SAMPLES})
    add_subdirectory(samples)
endif()

install(FILES ${HEADERS}
        DESTINATION ${CMAKE_INSTALL_PREFIX})

