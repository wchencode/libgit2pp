#include "Wrapper.h"
#include <stdexcept>

#include <unistd.h>

using namespace std;
using namespace libgit2pp;

void testCreateGitRepository() {
  const string root("/tmp/testCreateGitRepository");
  unlink(root.c_str());

  // Initializing libgit2 library.
  Git2 git2;

  try {
    // Create a bare repository.
    Repository r(root, true);
  } catch (const exception& ex) {
    throw runtime_error("Fails to create a new git repository");
  }

  // Test that we can open the new repository.
  try {
    Repository r(root);
  } catch (const exception& ex) {
    throw runtime_error("Fails to open a newly created repository");
  }
}

main() {
  testCreateGitRepository();
}
