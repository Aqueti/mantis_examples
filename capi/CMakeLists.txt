cmake_minimum_required(VERSION 2.8.12)

project(mantis_examles)

option( MAKE_TESTS "Make Mantis tests" ON )
option( MAKE_AUTOFOCUS "Make autofocus module" OFF )
set(SUPERBUILDING OFF CACHE BOOL "Currently Superbuilding")

list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake
    ${CMAKE_SOURCE_DIR}/cmake
    ${CMAKE_INSTALL_PREFIX}/cmake
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11")

if(${MAKE_AUTOFOCUS})
    find_package(OpenCV COMPONENTS core highgui imgcodecs imgproc)
    if(NOT "${OpenCV_FOUND}" AND NOT ${SUPERBUILDING})
        message("superbuilding now")
        set(SUPERBUILDING ON)
        include(ExternalProject)
        include(Superbuild.cmake)
        return()
    endif()
endif()

if(${MAKE_TESTS})
    find_package(MantisAPI REQUIRED)
    find_package(Threads REQUIRED)

    set(EXAMPLE_TARGETS
        HelloMantis
        GetClipList
        GetClipMcamImages
        MantisGetFrames
        MantisRecord
        McamStream
        SaveClip
    )

    add_executable(HelloMantis
        basic/HelloMantis.c
    )
    target_link_libraries(HelloMantis
        ${MantisAPI_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )

    add_executable(GetClipList
        basic/GetClipList.c
    )
    target_link_libraries(GetClipList
        ${MantisAPI_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )

    add_executable(GetClipMcamImages
        basic/GetClipMcamImages.c
    )
    target_link_libraries(GetClipMcamImages
        ${MantisAPI_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )

    add_executable(MantisGetFrames
        basic/MantisGetFrames.c
    )
    target_link_libraries(MantisGetFrames
        ${MantisAPI_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )

    add_executable(MantisRecord
        basic/MantisRecord.c
    )
    target_link_libraries(MantisRecord
        ${MantisAPI_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )

    add_executable(McamStream
        basic/McamStream.c
    )
    target_link_libraries(McamStream
        ${MantisAPI_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )

    add_executable(SaveClip
        basic/SaveClip.c
    )
    target_link_libraries(SaveClip
        ${MantisAPI_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )

    add_executable(MantisSetExposures
        globalWB/MantisSetExposures.c
    )
    target_link_libraries(MantisSetExposures
        ${MantisAPI_LIBRARIES}
        ${OpenCV_LIBS}
        ${CMAKE_THREAD_LIBS_INIT}
    )

endif()

if(${MAKE_AUTOFOCUS})
    message("opencv found: ${OpenCV_FOUND}")
    if(${OpenCV_FOUND})
        message("opencv: ${OpenCV_INCLUDE_DIR}")
        message("include: ${CMAKE_INCLUDE_PATH}")
        include_directories(include ${OpenCV_INCLUDE_DIR})
        include_directories(${CMAKE_INSTALL_PREFIX}/include)
        add_executable(Evetar_AutoFocus
            focus/Evetar_AutoFocus.cpp
        )
        target_link_libraries(Evetar_AutoFocus
            ${MantisAPI_LIBRARIES}
            ${OpenCV_LIBS}
            ${CMAKE_THREAD_LIBS_INIT}
        )
        list(APPEND EXAMPLE_TARGETS Evetar_AutoFocus)
    endif(${OpenCV_FOUND})
endif(${MAKE_AUTOFOCUS})

install(TARGETS ${EXAMPLE_TARGETS}
        RUNTIME DESTINATION bin
)
