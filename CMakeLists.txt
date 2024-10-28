cmake_minimum_required(VERSION 3.3)
#set(CMAKE_C_COMPILER "gcc-7")
#set(CMAKE_CXX_COMPILER "g++-7")
project(cosoco)

# Check if the 'no-xcsp3' option is enabled
option(NO_XCSP3 "Disable XCSP3 support" OFF)

# define some dirs
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/main")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# define some c++ flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wno-parentheses -Wall -O3 -Wno-unused-label")


include_directories(${CMAKE_CURRENT_SOURCE_DIR})
if(NOT NO_XCSP3)
    #if we're not using xcsp, don't include libxml2
    include_directories(/usr/local/opt/libxml2/include/libxml2/)
    find_package(LibXml2 REQUIRED)
    include_directories(${LIBXML2_INCLUDE_DIRS})
endif()

file(GLOB_RECURSE Cosoco_SOURCES
        ${PROJECT_SOURCE_DIR}/constraints/*.cc
        ${PROJECT_SOURCE_DIR}/core/*.cc
        ${PROJECT_SOURCE_DIR}/mtl/*.cc
        ${PROJECT_SOURCE_DIR}/optimizer/*.cc
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
)

add_library(${LIBRARY_NAME} STATIC ${Cosoco_SOURCES}
        solver/utils/Profiling.cc)

if(NOT NO_XCSP3)
    include_directories(
            ${CMAKE_CURRENT_SOURCE_DIR}/../XCSP3-CPP-Parser/include/
    )

    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../XCSP3-CPP-Parser/lib)
    target_compile_definitions(${LIBRARY_NAME} PRIVATE USE_XCSP3)
endif()

set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME "cosoco")

if(NOT NO_XCSP3)
    link_libraries(xcsp3parser)
    add_executable(cosoco main/Main.cc
            mtl/Matrix.h)
    target_compile_definitions(cosoco PRIVATE USE_XCSP3)
    target_link_libraries(cosoco ${LIBRARY_NAME})
    target_link_libraries(cosoco ${LIBXML2_LIBRARIES})
    
    
    target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()
