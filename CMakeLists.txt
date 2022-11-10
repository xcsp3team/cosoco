cmake_minimum_required(VERSION 3.3)
#set(CMAKE_C_COMPILER "gcc-7")
#set(CMAKE_CXX_COMPILER "g++-7")
project(cosoco)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/main")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wno-parentheses -Wall -O3 -Wno-unused-label")

include_directories(/usr/local/opt/libxml2/include/libxml2/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
#   set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /usr/local/externalsources)

find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIRS})

#set(LIBS ${LIBS} xml2)
#target_link_libraries(xml2 cosoco)
file(GLOB_RECURSE Cosoco_SOURCES
        ${PROJECT_SOURCE_DIR}/constraints/*.cc
        ${PROJECT_SOURCE_DIR}/core/*.cc
        ${PROJECT_SOURCE_DIR}/mtl/*.cc
        ${PROJECT_SOURCE_DIR}/optimizer/*.cc
        ${PROJECT_SOURCE_DIR}/problems/*.cc
        ${PROJECT_SOURCE_DIR}/satwrapper/*.cc
        ${PROJECT_SOURCE_DIR}/solver/*.cc
        ${PROJECT_SOURCE_DIR}/utils/*.cc
        ${PROJECT_SOURCE_DIR}/pFactory/*.cc
        ${PROJECT_SOURCE_DIR}/parallelsolver/*.cc)


file(GLOB_RECURSE Cosoco_HEADERS
        ${PROJECT_SOURCE_DIR}/constraints/*.h
        ${PROJECT_SOURCE_DIR}/core/*.h
        ${PROJECT_SOURCE_DIR}/mtl/*.h
        ${PROJECT_SOURCE_DIR}/main/*.h
        ${PROJECT_SOURCE_DIR}/optimizer/*.h
        ${PROJECT_SOURCE_DIR}/problems/*.h
        ${PROJECT_SOURCE_DIR}/satwrapper/*.h
        ${PROJECT_SOURCE_DIR}/solver/*.h
        ${PROJECT_SOURCE_DIR}/utils/*.h
        ${PROJECT_SOURCE_DIR}/pFactory/*.h
        ${PROJECT_SOURCE_DIR}/parallelsolver/*.h
        )

set(LIBRARY_NAME libcosoco)


#        file(GLOB_RECURSE Cosoco_SOURCES "./*.cc")
file(GLOB_RECURSE Cosoco_HEADERS "./*.h")

set(Cosoco_INCLUDE_DIRS "")

foreach (_headerFile ${Cosoco_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list(APPEND Cosoco_INCLUDE_DIRS ${_dir})
endforeach ()

list(REMOVE_DUPLICATES Cosoco_INCLUDE_DIRS)

include_directories(
        ${Cosoco_INCLUDE_DIRS}
        #        ${CMAKE_CURRENT_SOURCE_DIR}/cmake-build-debug/parser/src/XCSP3-CPP-Parser/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/../XCSP3-CPP-Parser/include/
)


add_library(${LIBRARY_NAME} STATIC ${Cosoco_SOURCES})


link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../XCSP3-CPP-Parser/lib)

set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME "cosoco")


link_libraries(xcsp3parser)
link_libraries(pthread)
add_executable(cosoco main/Main.cc)
target_link_libraries(cosoco ${LIBRARY_NAME})
target_link_libraries(cosoco ${LIBXML2_LIBRARIES})

#add_executable(test test/test.cc)
#target_link_libraries(test ${LIBRARY_NAME})
#target_link_libraries(test ${LIBXML2_LIBRARIES})
target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
