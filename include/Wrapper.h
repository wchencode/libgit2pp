#pragma once

#include "git2.h"
#include <string>

namespace libgit2pp {

// A wrapper class to initiating libgit2 library.
class Git2 {
 public:
  Git2() {
    git_libgit2_init();
  }

  ~Git2() {
    git_libgit2_shutdown();
  }
};

// A wrapper class for git_repository.
class Repository {
 public:
  // Create a wrapper class for git_repository.
  explicit Repository(git_repository* repo);

  // Open an existing repository, throws an exception if path does not
  // contain a repository.
  explicit Repository(const std::string& path);

  /*
   Create a new repository in the given folder.

   @param path the path to the repository
   @param is_bare if true, a Git repository without a working directory is
          created at the pointed path. If false, provided path will be
          considered as the working directory into which the .git directory
          will be created.

   Throws an exception if the repository cannot be created.
  */
  Repository(const std::string& path, bool isBare);

  // Moving constructor.
  Repository(Repository&& b);

  ~Repository();

  git_repository* get() { return repo_; }

  // Returns git_repository pointer. The caller needs to
  // release it later.
  git_repository* release();

 private:
  git_repository* repo_;
};

}  // libgit2pp
