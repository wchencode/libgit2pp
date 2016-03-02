add_executable(testRepository RepositoryTest.cpp)
target_include_directories(
    testRepository PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${LIBGIT2_INCLUDE_DIR})
target_link_libraries(
  testRepository LINK_PUBLIC
  git2pp
  ${LIBGIT2_LIBRARY}
  -pthread
  -lssl -lcrypto
  -lz
)

add_executable(testTree TreeTest.cpp)
target_include_directories(
    testTree PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${LIBGIT2_INCLUDE_DIR})
target_link_libraries(
  testTree LINK_PUBLIC
  git2pp
#  -L${CMAKE_BINARY_DIR}/src -lgit2pp
  ${LIBGIT2_LIBRARY}
  -pthread
  -lssl -lcrypto
  -lz
)

add_executable(testCommit CommitTest.cpp)
target_include_directories(
    testCommit PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${LIBGIT2_INCLUDE_DIR})
target_link_libraries(
  testCommit LINK_PUBLIC
  git2pp
  ${LIBGIT2_LIBRARY}
  -pthread
  -lssl -lcrypto
  -lz
)

add_executable(testUserCommit UserCommitTest.cpp)
target_include_directories(
    testUserCommit PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${LIBGIT2_INCLUDE_DIR})
target_link_libraries(
  testUserCommit LINK_PUBLIC
  git2pp
  ${LIBGIT2_LIBRARY}
  -pthread
  -lssl -lcrypto
  -lz
)
