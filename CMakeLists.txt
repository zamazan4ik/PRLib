cmake_minimum_required(VERSION 3.2.0)
project(prlib)

include_directories(src/*/* src/binarizations src/balance src/border_detection src/deblur src/denoise src/deskew
                    src/detectors src/thinning)

include_directories( ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/src)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 11)

file(GLOB_RECURSE HEADERS "src/*/*.h" "src/*.h")
file(GLOB_RECURSE SOURCES "src/*/*.cpp" "src/*.cpp" "src/*/*.h" "src/*.h")

foreach(_header ${HEADERS})
    file(COPY ${_header} DESTINATION ${CMAKE_BINARY_DIR}/include)
endforeach()

find_package(Leptonica)
find_package(OpenCV)

include_directories(${Leptonica_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})

add_library(prlib STATIC ${SOURCES})

target_link_libraries(prlib
        ${OpenCV_LIBS}
        ${Leptonica_LIBRARIES})

# Samples

# Binarization samples
add_executable(binarizeCOCOCLUST_sample samples/binarizations/binarizeCOCOCLUST_sample.cpp)
target_link_libraries(binarizeCOCOCLUST_sample prlib)

add_executable(binarizeFBCITB_sample samples/binarizations/binarizeFBCITB_sample.cpp)
target_link_libraries(binarizeFBCITB_sample prlib)

add_executable(binarizeFeng_sample samples/binarizations/binarizeFeng_sample.cpp)
target_link_libraries(binarizeFeng_sample prlib)

add_executable(binarizeLocalOtsu_sample samples/binarizations/binarizeLocalOtsu_sample.cpp)
target_link_libraries(binarizeLocalOtsu_sample prlib)

add_executable(binarizeNativeAdaptive_sample samples/binarizations/binarizeNativeAdaptive_sample.cpp)
target_link_libraries(binarizeNativeAdaptive_sample prlib)

add_executable(binarizeNiblack_sample samples/binarizations/binarizeNiblack_sample.cpp)
target_link_libraries(binarizeNiblack_sample prlib)

add_executable(binarizeNICK_sample samples/binarizations/binarizeNICK_sample.cpp)
target_link_libraries(binarizeNICK_sample prlib)

add_executable(binarizeSauvola_sample samples/binarizations/binarizeSauvola_sample.cpp)
target_link_libraries(binarizeSauvola_sample prlib)

add_executable(binarizeWolfJolion_sample samples/binarizations/binarizeWolfJolion_sample.cpp)
target_link_libraries(binarizeWolfJolion_sample prlib)


# Thinning samples
add_executable(thinGuoHall_sample samples/thinning/thinGuoHall_sample.cpp)
target_link_libraries(thinGuoHall_sample prlib)

add_executable(thinZhangSuen_sample samples/thinning/thinZhangSuen_sample.cpp)
target_link_libraries(thinZhangSuen_sample prlib)


# Deblur samples
add_executable(basicDeblur_sample samples/deblur/basicDeblur_sample.cpp)
target_link_libraries(basicDeblur_sample prlib)

add_executable(wienerFilter_sample samples/deblur/wienerFilter_sample.cpp)
target_link_libraries(wienerFilter_sample prlib)


# Normalization samples
add_executable(backgroundNormalization_sample samples/backgroundNormalization_sample.cpp)
target_link_libraries(backgroundNormalization_sample prlib)


# Denoise samples
add_executable(kuwaharaFilter_sample samples/denoise/kuwaharaFilter_sample.cpp)
target_link_libraries(kuwaharaFilter_sample prlib)

# Border detection samples

# Correct non-uniform illumination samples
add_executable(correctNUIL_sample samples/correctNUIL_sample.cpp)
target_link_libraries(correctNUIL_sample prlib)

# Remove hole punches samples
add_executable(removeHolePunch_sample samples/removeHolePunch_sample.cpp)
target_link_libraries(removeHolePunch_sample prlib)

# Remove lines samples
add_executable(removeLines_sample samples/removeLines_sample.cpp)
target_link_libraries(removeLines_sample prlib)

# Remove dots sample
add_executable(removeDots_sample samples/removeDots_sample.cpp)
target_link_libraries(removeDots_sample prlib)

# Warp sample
add_executable(warp_sample samples/warp_sample.cpp)
target_link_libraries(warp_sample prlib)

# Auto crop sample
add_executable(autoCrop_sample samples/border_detection/autoCrop_sample.cpp)
target_link_libraries(autoCrop_sample prlib)




