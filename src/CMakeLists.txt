add_library(
  git2pp STATIC
  Wrapper.cpp
  TestUtils.cpp
)
target_include_directories(
  git2pp PUBLIC
  ${CMAKE_SOURCE_DIR}/include
  ${LIBGIT2_INCLUDE_DIR}
)
target_link_libraries(
  git2pp LINK_PUBLIC
  ${LIBGIT2_LIBRARY}
)
