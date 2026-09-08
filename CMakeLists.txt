cmake_minimum_required(VERSION 3.10)
#set(CMAKE_C_COMPILER "gcc-7")
#set(CMAKE_CXX_COMPILER "g++-7")
project(cosoco)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/main")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wno-parentheses -Wall -O3 -Wno-unused-label")

option(STATIC_BUILD "Produce a portable binary by linking dependencies statically" OFF)

if(APPLE)
    # Homebrew keeps libxml2 keg-only, so it is not in the default search path.
    # /opt/homebrew: Apple Silicon -- /usr/local: Intel
    foreach(_brew_prefix /opt/homebrew /usr/local)
        if(EXISTS ${_brew_prefix}/opt/libxml2)
            list(APPEND CMAKE_PREFIX_PATH ${_brew_prefix}/opt/libxml2)
        endif()
    endforeach()
endif()

if(STATIC_BUILD)
    if(APPLE AND NOT CMAKE_OSX_DEPLOYMENT_TARGET)
        # Governs which macOS versions the binary runs on: the counterpart of
        # the glibc version constraint on Linux.
        set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version" FORCE)
    endif()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(STATIC_BUILD)
    # Select libxml2.a rather than the shared library. Restricted to this
    # lookup: on macOS the system libraries below exist only as dylibs.
    set(_saved_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
endif()

find_package(LibXml2 REQUIRED)

if(STATIC_BUILD)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${_saved_suffixes})
endif()

include_directories(${LIBXML2_INCLUDE_DIRS})

file(GLOB_RECURSE Cosoco_SOURCES
        ${PROJECT_SOURCE_DIR}/constraints/*.cc
        ${PROJECT_SOURCE_DIR}/core/*.cc
        ${PROJECT_SOURCE_DIR}/mtl/*.cc
        ${PROJECT_SOURCE_DIR}/optimizer/*.cc
        ${PROJECT_SOURCE_DIR}/parallelsolver/*.cc
        ${PROJECT_SOURCE_DIR}/problems/*.cc
        ${PROJECT_SOURCE_DIR}/solver/*.cc
        ${PROJECT_SOURCE_DIR}/utils/*.cc
)

file(GLOB_RECURSE Cosoco_HEADERS
        ${PROJECT_SOURCE_DIR}/constraints/*.h
        ${PROJECT_SOURCE_DIR}/core/*.h
        ${PROJECT_SOURCE_DIR}/mtl/*.h
        ${PROJECT_SOURCE_DIR}/main/*.h
        ${PROJECT_SOURCE_DIR}/optimizer/*.h
        ${PROJECT_SOURCE_DIR}/parallelsolver/*.h
        ${PROJECT_SOURCE_DIR}/problems/*.h
        ${PROJECT_SOURCE_DIR}/solver/*.h
        ${PROJECT_SOURCE_DIR}/utils/*.h
)

set(LIBRARY_NAME libcosoco)


file(GLOB_RECURSE Cosoco_HEADERS "./*.h")

set(Cosoco_INCLUDE_DIRS "")

foreach (_headerFile ${Cosoco_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list(APPEND Cosoco_INCLUDE_DIRS ${_dir})
endforeach ()

list(REMOVE_DUPLICATES Cosoco_INCLUDE_DIRS)

include_directories(
        ${Cosoco_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/../XCSP3-CPP-Parser/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/../pfactory/include/

)


add_library(${LIBRARY_NAME} STATIC ${Cosoco_SOURCES})


link_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/../XCSP3-CPP-Parser/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/../pfactory/lib
)

set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME "cosoco")


link_libraries(
        xcsp3parser
        pthread
        pFactory
)
link_libraries(pthread)

add_executable(cosoco main/Main.cc)
target_link_libraries(cosoco ${LIBRARY_NAME})
target_link_libraries(cosoco ${LIBXML2_LIBRARIES})

if(STATIC_BUILD)
    # libxml2 needs these; they must come after it on the link line.
    target_link_libraries(cosoco z)
    if(APPLE)
        # macOS ships no static libSystem, so a fully static executable is not
        # possible there. Dependencies are linked statically and the system
        # libraries (libSystem, libc++, libiconv) stay dynamic -- they are part
        # of macOS itself, so the binary still needs nothing installed.
        target_link_libraries(cosoco iconv)
        # liblzma is not always offered as a link stub by the macOS SDK.
        find_library(LZMA_LIBRARY lzma)
        if(LZMA_LIBRARY)
            target_link_libraries(cosoco ${LZMA_LIBRARY})
        endif()
    else()
        target_link_libraries(cosoco lzma)
        set_target_properties(cosoco PROPERTIES LINK_FLAGS "-static")
    endif()
endif()


target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

