set(cmake_common_args
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_C_COMPILER:PATH=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER:PATH=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/INSTALL
    -DCMAKE_INSTALL_RPATH:PATH=${CMAKE_BINARY_DIR}/INSTALL/lib${LIB_SUFFIX}
    -DCMAKE_INCLUDE_PATH:PATH=${CMAKE_INSTALL_PREFIX}/include
    -DMAKE_TESTS:BOOL=${MAKE_TESTS}
    -DMAKE_AUTOFOCUS:BOOL=${MAKE_AUTOFOCUS}
    -DSUPERBUILDING:BOOL=${SUPERBUILDING}
)

if(${MAKE_AUTOFOCUS})
    ExternalProject_Add(
        OpenCV
        GIT_REPOSITORY "https://github.com/opencv/opencv.git"
        GIT_TAG "3.2.0"
        CMAKE_ARGS 
            -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/INSTALL
            -DCMAKE_BUILD_TYPE=RELEASE
            -DINSTALL_C_EXAMPLES=OFF
            -DINSTALL_PYTHON_EXAMPLES=OFF
            #-DOPENCV_EXTRA_MODULES_PATH=~/opencv_contrib/modules
            -DBUILD_EXAMPLES=OFF
        INSTALL_DIR ${CMAKE_BINARY_DIR}/INSTALL
    )
endif()

#mantis_examples
ExternalProject_Add(mantis_examples
    SOURCE_DIR ${CMAKE_SOURCE_DIR}
    BUILD_ALWAYS 1
    CMAKE_ARGS 
        ${cmake_common_args} 
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    DEPENDS OpenCV
)
