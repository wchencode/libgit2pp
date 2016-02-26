#include "Wrapper.h"
#include "TestUtils.h"

#include <stdexcept>
#include <sstream>
#include <string>
#include <memory>

#include <unistd.h>

using namespace std;
using namespace libgit2pp;

void testCreateNewTree() {
  const string root("/tmp/testCreateNewTree");
  setupRoot(root);

  // Initializing libgit2 library.
  Git2 git2;

  unique_ptr<Repository> r;

  // Create a bare repository.
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

  // Manage the returned tree.
  Tree t(tree);
  if (1 != git_tree_entrycount(tree)) {
    throw runtime_error("Tree should have only one entry");
  }
  // There is no need to release @param entry.
  auto entry = git_tree_entry_byname(tree, "README");
  if (entry == nullptr) {
    throw runtime_error("Fails to find filename in the tree");
  }
  const git_oid* retId = git_tree_entry_id(entry);
  if (retId == nullptr) {
    throw runtime_error("Fails to retrieve object ID");
  }
  if (0 != git_oid_cmp(&id, retId)) {
    throw runtime_error("Expect to retrieve original object ID");
  }
}

main() {
  testCreateNewTree();
}
