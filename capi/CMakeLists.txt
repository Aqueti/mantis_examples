cmake_minimum_required(VERSION 2.8.12)

project(mantis_examles)

option( MAKE_TESTS "Make Mantis tests" ON )

list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_SOURCE_DIR}/cmake
    ${CMAKE_INSTALL_PREFIX}/cmake
)

if(${MAKE_TESTS})
    find_package(MantisAPI REQUIRED)
    find_package(Threads REQUIRED)

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

    install(TARGETS 
        HelloMantis
        GetClipList
        GetClipMcamImages
        MantisGetFrames
        MantisRecord
        McamStream
        SaveClip
        RUNTIME DESTINATION bin
    )
endif()
