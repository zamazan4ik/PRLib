cmake_minimum_required(VERSION 3.6.0)
project(prlib)

set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS}" )

include_directories(include/opencv include/opencv/opencv2 include/spdlog)

include_directories(src/*/* src/binarizations src/balance src/border_detection src/deblur src/denoise src/deskew
                    src/detectors src/thinning)

link_directories(lib/opencv lib/libjpeg lib/libpng lib/libtiff lib/zlib lib/libjasper
        lib/libwebp lib/libippcv lib/libprotobuf lib/leptonica lib/libgif)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 14)

#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

set(OpenCV_LIBSS libopencv_ximgproc.a libopencv_xphoto.a libopencv_shape.a libopencv_imgcodecs.a libopencv_ts.a
        libopencv_videostab.a libopencv_photo.a libopencv_calib3d.a
        libopencv_video.a libopencv_stitching.a
        libopencv_videoio.a libopencv_superres.a
        libopencv_ml.a libopencv_objdetect.a  libopencv_features2d.a
        libopencv_flann.a libopencv_imgproc.a libopencv_highgui.a libopencv_core.a)

file(GLOB_RECURSE HEADERS "src/*/*.h" "src/*.h")
file(GLOB_RECURSE SOURCES "src/*/*.cpp" "src/*.cpp" "src/*/*.h" "src/*.h")


foreach(_header ${HEADERS})
    file(COPY ${_header} DESTINATION ${CMAKE_BINARY_DIR}/include)
endforeach()



set(CMAKE_BUILD_TYPE Debug)

#add_subdirectory(samples)

include_directories( ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/src)

find_package(OpenCV)

add_library(prlib STATIC ${SOURCES})

target_link_libraries(prlib
        ${OpenCV_LIBS}
        -llept
        #-fPIC
        #liblibjpeg.a
        #[[-lzlib
        -ltiff.a
        -lippicv.a
        liblibwebp.a
        liblibjasper.a

        -ljpeg
        -lpng
        -lgif

        -lgomp
        -fPIC
        -lpthread]])

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



# Normalization samples
add_executable(backgroundNormalization_sample samples/backgroundNormalization_sample.cpp)
target_link_libraries(backgroundNormalization_sample prlib)


# Border detection samples
add_executable(autoCropAbbyy_sample samples/border_detection/autoCropAbbyy_sample.cpp)
target_link_libraries(autoCropAbbyy_sample prlib)

#add_subdirectory(samples)

#add_executable(prlib samples/binarizations/binarizeCOCOCLUST_sample.cpp)
#target_link_libraries(binCOCO prlib)

#[[target_link_libraries(prlib
        ${OpenCV_LIBSS}
        libleptonica.a

        #liblibjpeg.a
        libzlib.a
        liblibtiff.a
        libippicv.a
        liblibwebp.a
        liblibjasper.a

        libjpeg.a
        liblibpng.a
        libgif.a

        -lgomp
        -fPIC
        -lpthread)]]



