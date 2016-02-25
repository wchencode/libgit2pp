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

git_repository* Repository::release() {
  auto ret = repo_;
  repo_ = nullptr;
  return ret;
}

}
