CMake_minimum_required(VERSION 2.8.12)

project(mantis_examles)

option( BUILD_TESTS "Make Mantis tests" ON )

list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_SOURCE_DIR}/cmake
    /etc/aqueti/cmake
)
list(APPEND CMAKE_PREFIX_PATH
    ${CMAKE_INSTALL_PREFIX}/lib/cmake
    /usr/lib/cmake
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(MantisAPI CONFIG REQUIRED)
find_package(Threads REQUIRED)

if(${BUILD_TESTS})
    include_directories(${CMAKE_INSTALL_PREFIX}/include)

    set(EXAMPLE_TARGETS
        HelloMantis
        GetClipList
        GetClipMcamImages
        MantisGetFrames
        MantisRecord
        McamStream
        SaveClip
        MantisSetExposures
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
endif()

install(TARGETS ${EXAMPLE_TARGETS}
        RUNTIME DESTINATION bin
)
