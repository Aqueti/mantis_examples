cmake_minimum_required(VERSION 2.8.12)

project(mantis_examles)

option( MAKE_TESTS "Make Mantis tests" ON )
option( MAKE_AUTOFOCUS "Make autofocus module" OFF )

list(APPEND CMAKE_PREFIX_PATH
    ${CMAKE_INSTALL_PREFIX}/lib/cmake
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if(${MAKE_TESTS})
    find_package(MantisAPI CONFIG REQUIRED)
    find_package(Threads REQUIRED)

    include_directories(${CMAKE_INSTALL_PREFIX}/include)

    set(EXAMPLE_TARGETS
        HelloMantis
        GetClipList
        GetClipMcamImages
        MantisGetFrames
        MantisRecord
        McamStream
        SaveClip
    )

    foreach(target ${EXAMPLE_TARGETS})
        add_executable(${target}
            basic/${target}.c
        )
        target_link_libraries(${target}
            MantisAPI
            Threads::Threads
        )
    endforeach(target)

    add_executable(MantisSetExposures
        globalWB/MantisSetExposures.c
    )
    target_link_libraries(MantisSetExposures
        MantisAPI
        ${OpenCV_LIBS}
        Threads::Threads
    )
    list(APPEND EXAMPLE_TARGETS MantisSetExposures)

endif()

if(${MAKE_AUTOFOCUS})
    set(OpenCV_DIR /usr/local/share/OpenCV)
    find_package(OpenCV REQUIRED COMPONENTS core highgui imgcodecs imgproc)
    include_directories(include ${OpenCV_INCLUDE_DIR})
    add_executable(Evetar_AutoFocus
        focus/Evetar_AutoFocus.cpp
    )
    target_link_libraries(Evetar_AutoFocus
        MantisAPI
        ${OpenCV_LIBRARIES}
        Threads::Threads
    )
    list(APPEND EXAMPLE_TARGETS Evetar_AutoFocus)
endif(${MAKE_AUTOFOCUS})

install(TARGETS ${EXAMPLE_TARGETS}
        RUNTIME DESTINATION bin
)
