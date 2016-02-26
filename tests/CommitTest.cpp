#include "Wrapper.h"
#include "TestUtils.h"

#include <stdexcept>
#include <sstream>
#include <string>
#include <memory>

#include <unistd.h>

using namespace std;
using namespace libgit2pp;

void testNewCommit() {
  const string root("/tmp/testNewCommit");
  setupRoot(root);

  // Initializing libgit2 library.
  Git2 git2;

  // Create a bare repository.
  unique_ptr<Repository> r;
  try {
    r = make_unique<Repository>(root, true);
  } catch (const exception& ex) {
    throw runtime_error("Fails to create a new git repository");
  }

  // Prepare the first file in the repository.
  auto tmppath = root + "/tmpfile";
  writeToFile(tmppath, "Hello, world\n");

  // Write the contents into object database.
  git_oid id;
  if (!r->createBlobFromDisk(tmppath, &id)) {
    throw runtime_error("Fails to create an object in git");
  }
  unlink(tmppath.c_str());

  // Build a brand new tree.
  auto builder = r->createTreeBuilder(nullptr);
  if (builder == nullptr) {
    throw runtime_error("Fails to create a new treebuilder");
  }
  TreeBuilder b(builder);
  if (!b.insert("README", &id, (git_filemode_t)0100644)) {
    throw runtime_error("Fails to insert a new file into tree");
  }
  git_oid tid;
  if (!b.write(&tid)) {
    throw runtime_error("Fails to create a new tree");
  }

  // Lookup the tree.
  auto tree = r->getTree(&tid);
  if (!tree) {
    throw runtime_error("Fails to retrieve a created tree");
  }

  // Commit a change.
  git_oid commitId;
  if (!r->commit(
          &commitId,
          "HEAD",
          "My Name",
          "myname@gmail.com",
          "My first commit",
          tree,
          0, /*size_t parentCount*/
          nullptr /*const git_commit *parents[]*/)) {
    throw runtime_error("Fails to commit");
  }
}

main() {
  testNewCommit();
}
