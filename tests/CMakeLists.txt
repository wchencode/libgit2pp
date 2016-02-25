add_executable(testRepository RepositoryTest.cpp)
target_include_directories(
    testRepository PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${LIBGIT2_INCLUDE_DIR})
target_link_libraries(
  testRepository LINK_PUBLIC
  -L${CMAKE_BINARY_DIR}/src -lgit2pp
  ${LIBGIT2_LIBRARY}
  -pthread
  -lssl -lcrypto
  -lz
)
