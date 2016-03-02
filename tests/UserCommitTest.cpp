#include "Wrapper.h"
#include <stdexcept>
#include <unordered_map>
#include <iostream>

#include <unistd.h>

using namespace std;
using namespace libgit2pp;

void testUserCommit() {
  const string root("/tmp/testUserCommit");
  unlink(root.c_str());

  // Initializing libgit2 library.
  Git2 git2;
  unique_ptr<Repository> r;

  try {
    // Create a bare repository.
    r.reset(new Repository(root, true));
  } catch (const exception& ex) {
    throw runtime_error("Fails to create a new git repository");
  }

  unordered_map<string, string> addedFiles = {
    {"a/b/Foo.h", "struct Foo {};"},
    {"README", "hello, world"},
    {"a/Bar.h", "struct Bar{};"},
    {"x/Makefile", "Make something"},
    {"a/README", "hello, world"},
  };

  string id = r->commit(
      "",
      "My Name",
      "my.name@gmail.com",
      "A testing commit",
      addedFiles,
      unordered_set<string>());

  if (id.empty()) {
    throw runtime_error("Fails to create a commit");
  } else {
    cout << "New commit is " << id << endl;
  }
}

main() {
  testUserCommit();
}
