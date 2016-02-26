#include "Wrapper.h"
#include "TestUtils.h"

#include <stdexcept>
#include <sstream>
#include <string>
#include <memory>
#include <iostream>

#include <unistd.h>

using namespace std;
using namespace libgit2pp;

const string root("/tmp/testCommit");

void testFirstCommit() {
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

void testMoreCommit() {
  // Initializing libgit2 library.
  Git2 git2;

  // Open an existing repository.
  unique_ptr<Repository> r;
  try {
    r = make_unique<Repository>(root);
  } catch (const exception& ex) {
    throw runtime_error("Fails to open a git repository");
  }

  // Prepare a new file in the repository.
  auto tmppath = root + "/tmpfile";
  writeToFile(tmppath, "#include <iostream>\n");

  // Write the contents into object database.
  git_oid id;
  if (!r->createBlobFromDisk(tmppath, &id)) {
    throw runtime_error("Fails to create an object in git");
  }
  unlink(tmppath.c_str());

  // Get the head.
  git_reference* gr = r->getHead();
  if (gr == nullptr) {
    throw runtime_error("Fails to find the HEAD");
  }
  Reference ref(gr);
  const git_oid* target = git_reference_target(gr);
  if (target == nullptr) {
    throw runtime_error("Fails to find target");
  }

  // Verify target's type.
  auto odb = r->getOdb();
  Odb o(odb);
  size_t len = 0;
  git_otype type;
  if (0 != git_odb_read_header(&len, &type, odb, target)) {
    throw runtime_error("Fails to read object from ODB");
  }
  if (type != GIT_OBJ_COMMIT) {
    throw runtime_error("Expect a commit object");
  }

  // Retrieve existing tree.
  git_commit* c = r->getCommit(target);
  if (c == nullptr) {
    throw runtime_error("Fails to retrieve a commit");
  }
  Commit commit(c);
  git_tree* existingTree = nullptr;
  if (0 != git_commit_tree(&existingTree, c)) {
    throw runtime_error("Fails to get existing tree");
  }
  Tree treeHolder(existingTree);

  // Build a new tree.
  auto builder = r->createTreeBuilder(existingTree);
  if (builder == nullptr) {
    throw runtime_error("Fails to create a new treebuilder");
  }
  TreeBuilder b(builder);
  if (!b.insert("File", &id, (git_filemode_t)0100644)) {
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
  const git_commit* parents[] = {c};
  if (!r->commit(
          &commitId,
          "HEAD",
          "My Name",
          "myname@gmail.com",
          "My second commit",
          tree,
          1, /*size_t parentCount*/
          parents /*const git_commit *parents[]*/)) {
    throw runtime_error("Fails to commit");
  }
}

main() {
  testFirstCommit();
  testMoreCommit();
}
