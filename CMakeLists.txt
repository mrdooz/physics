# must be run to find SDL2 on OSX
# export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig

cmake_minimum_required(VERSION 2.8)
project(physics)

include(FindProtobuf)
include(FindSDL2)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
find_package(Boost REQUIRED)
find_package(Protobuf REQUIRED)
find_package(SDL2 REQUIRED)

include_directories(${Boost_INCLUDE_DIR})
include_directories(${PROTOBUF_INCLUDE_DIR})

#include(FindPkgConfig)
#pkg_search_module(SDL2 REQUIRED sdl2)
include_directories(${SDL2_INCLUDE_DIR})

#installed via macports => /opt/local/include
include_directories(${GLEW_INCLUDE_DIR})

find_package(OpenGL)
find_package(GLEW)

file(GLOB SRC "*.cpp" "*.hpp" "protocol/*.pb.cc")
add_executable(${PROJECT_NAME} ${SRC})


if (APPLE)
  # change c++ standard library to libc++ (llvm)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v -std=c++11 -stdlib=libc++")
  set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
    XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/precompiled.hpp"
    XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES"
    LINK_FLAGS "-F/Library/Frameworks")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")

target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})

  # specifically link against a protobuf and boost build with libc++
  target_link_libraries(${PROJECT_NAME}
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    "/opt/local/boost/lib/libboost_date_time.a"
    "/opt/local/protobuf/lib/libprotobuf.a")
else()
  if (MSVC)
    # global all the root level .cpp files
    file(GLOB ROOT_SRC "*.cpp")

    # add precompiled header, and force include it on all the root level .cpp files
    foreach( src_file ${ROOT_SRC} )
        set_source_files_properties(${src_file} PROPERTIES COMPILE_FLAGS "/Yuprecompiled.hpp /FIprecompiled.hpp")
    endforeach( src_file ${ROOT_SRC} )

    set_source_files_properties(precompiled.cpp PROPERTIES COMPILE_FLAGS "/Ycprecompiled.hpp")

    # Force static runtime libraries
    foreach(flag CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_DEBUG)
      STRING(REPLACE "/MD"  "/MT" "${flag}" "${${flag}}")
      SET("${flag}" "${${flag}}")
    endforeach()

    target_link_libraries(pang
      debug ${SFML_GRAPHICS_LIBRARY_DEBUG} optimized ${SFML_GRAPHICS_LIBRARY_RELEASE}
      debug ${SFML_SYSTEM_LIBRARY_DEBUG} optimized ${SFML_SYSTEM_LIBRARY_RELEASE}
      debug ${SFML_WINDOW_LIBRARY_DEBUG} optimized ${SFML_WINDOW_LIBRARY_RELEASE}
      ${Boost_DATETIME_LIBRARY}
      debug ${PROTOBUF_LIBRARY_DEBUG} optimized ${PROTOBUF_LIBRARY})

  endif(MSVC)
endif()
