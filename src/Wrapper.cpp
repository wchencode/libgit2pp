#include "Wrapper.h"
#include <stdexcept>

using namespace std;

namespace libgit2pp {

Repository::Repository(git_repository* repo) : repo_(repo) {
}

Repository::Repository(const string& path) {
  if (0 != git_repository_open(&repo_, path.c_str())) {
    throw runtime_error("Fails to open a repository");
  }
}

Repository::Repository(const string& path, bool isBare) {
  if (0 != git_repository_init(&repo_, path.c_str(), isBare)) {
    throw runtime_error("Fails to create a repository");
  }
}

Repository::Repository(Repository&& b) {
  std::swap(repo_, b.repo_);
}

Repository::~Repository() {
  if (repo_) {
    git_repository_free(repo_);
  }
}

git_treebuilder* Repository::createTreeBuilder(const git_tree* source) {
  git_treebuilder* out = nullptr;
  if (0 == git_treebuilder_new(&out, repo_, source)) {
    return out;
  } else {
    return nullptr;
  }
}

git_tree* Repository::getTree(const git_oid* id) {
  git_tree* out = nullptr;
  if (0 == git_tree_lookup(&out, repo_, id)) {
    return out;
  } else {
    return nullptr;
  }
}

bool Repository::createBlobFromDisk(const std::string& path, git_oid* id) {
  return (0 == git_blob_create_fromdisk(id, repo_, path.c_str()));
}

git_repository* Repository::release() {
  auto ret = repo_;
  repo_ = nullptr;
  return ret;
}

}
