#include "Wrapper.h"
#include <stdexcept>
#include <sstream>
#include <string>
#include <memory>

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;
using namespace libgit2pp;

void setupRoot(const string& root) {
  // Remove leftover from previous runs.
  {
    stringstream ss;
    ss << "rm -rf " << root;
    system(ss.str().c_str());
  }

  // Create new directory.
  {
    stringstream ss;
    ss << "mkdir -p " << root;
    system(ss.str().c_str());
  }
}

void writeToFile(const string& path, const string& data) {
  int fd = open(path.c_str(), O_CREAT|O_WRONLY);
  if (fd < 0) {
    throw runtime_error("Fails to create a tmp file");
  }
  write(fd, data.c_str(), data.size());
  close(fd);
}

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
}

main() {
  testCreateNewTree();
}
