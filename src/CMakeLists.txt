add_library(
  git2pp STATIC
  Wrapper.cpp
  TestUtils.cpp
  PathTree.cpp
  DiffGenerator.cpp
)
target_include_directories(
  git2pp PUBLIC
  ${CMAKE_SOURCE_DIR}/include
  ${LIBGIT2_INCLUDE_DIR}
)
target_link_libraries(
  git2pp LINK_PUBLIC
  ${LIBGIT2_LIBRARY}
  -pthread
  -lssl -lcrypto
  -lz
)

add_executable(load_test LoadTest.cpp)
target_include_directories(
    load_test PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${LIBGIT2_INCLUDE_DIR})
target_link_libraries(
  load_test LINK_PUBLIC
  git2pp
  ${LIBGIT2_LIBRARY}
)
