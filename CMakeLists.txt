cmake_minimum_required (VERSION 2.8.11)
project(libgit2pp)

# Use g++-4.9 for compilation. Users may need to use environment variable
# CXX to point to the right location of g++-4.9
if ($ENV{CXX})
set(CMAKE_CXX_COMPILER $ENV{CXX})
endif ($ENV{CXX})

set(CMAKE_CXX_FLAGS "-std=c++1y -g")

# Try to find libgit2 
find_path(LIBGIT2_INCLUDE_DIR git2.h
          PATHS ${CMAKE_SOURCE_DIR}/../libgit2/include /usr/include /usr/include/libgit2)
find_library(LIBGIT2_LIBRARY NAMES git2
             PATHS ${CMAKE_SOURCE_DIR}/../libgit2/build /usr/lib /usr/libgit2)
message("libgit2 header is ${LIBGIT2_INCLUDE_DIR}")
message("libgit2 library is ${LIBGIT2_LIBRARY}")

add_subdirectory(src)
add_subdirectory(tests)
