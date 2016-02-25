add_executable(hello main.cpp)
target_include_directories(hello PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${lIBGIT2_INCLUDE_DIR})

add_library(
  git2pp STATIC
  Oid.cpp
  Wrapper.cpp
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
