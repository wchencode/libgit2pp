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

  /**
   Create a new tree builder.

   The tree builder can be used to create or modify trees in memory and
   write them as tree objects to the database.

   If the `source` parameter is not NULL, the tree builder will be
   initialized with the entries of the given tree.

   If the `source` parameter is NULL, the tree builder will start with no
   entries and will have to be filled manually.
  */
  git_treebuilder* createTreeBuilder(const git_tree* source);

  // Lookup a tree object from the repository.
  // @param id Identity of the tree to locate.
  git_tree* getTree(const git_oid* id);

  /**
   Read a file from the filesystem and write its content
   to the Object Database as a loose blob

   @param path file from which the blob will be created
   @param id return the id of the written blob
   @return true if there is no error.
  */
  bool createBlobFromDisk(const std::string& path, git_oid* id);

  git_repository* get() { return repo_; }

  // Returns git_repository pointer. The caller needs to
  // release it later.
  git_repository* release();

 private:
  git_repository* repo_;
};

// A wrapper class for git_tree_builder.
class TreeBuilder {
 public:
  // Construct a wrapper from the actual builder.
  explicit TreeBuilder(git_treebuilder* builder) : b_(builder) {
  }

  ~TreeBuilder() {
    if (b_) {
      git_treebuilder_free(b_);
    }
  }

  // Clear all the entires in the builder
  void clear() {
    git_treebuilder_clear(b_);
  }

  // Get the number of entries in the treebuilder.
  int entryCount() {
    return git_treebuilder_entrycount(b_);
  }

  /**
   Add or update an entry to the builder

   Insert a new entry for `filename` in the builder with the
   given attributes.

   If an entry named `filename` already exists, its attributes
   will be updated with the given ones.

   No attempt is being made to ensure that the provided oid points
   to an existing git object in the object database, nor that the
   attributes make sense regarding the type of the pointed at object.

   @param filename Filename of the entry
   @param id SHA1 oid of the entry
   @param filemode Folder attributes of the entry. This parameter must
          be valued with one of the following entries: 0040000, 0100644,
          0100755, 0120000 or 0160000.
  */
  const git_tree_entry* insert(
      const std::string& filename,
      const git_oid* id,
      git_filemode_t filemode) {
    const git_tree_entry* out = nullptr;
    if (0 == git_treebuilder_insert(
                 &out, b_, filename.c_str(), id, filemode)) {
      return out;
    } else {
      return nullptr;
    }
  }

  // Remove a file entry from treebuilder.
  void remove(const std::string& filename) {
    git_treebuilder_remove(b_, filename.c_str());
  }

  /**
   Write the contents of the tree builder as a tree object

   The tree builder will be written to the given `repo`, and its
   identifying SHA1 hash will be stored in the `id` pointer.

   @param id Pointer to store the OID of the newly written tree
   @return true if there is no error. 
  */
  bool write(git_oid* id) {
    return (0 == git_treebuilder_write(id, b_));
  }

 private:
  git_treebuilder* b_;
};

// Wrapper class for git_tree.
class Tree {
 public:
  explicit Tree(git_tree* t) : tree_(t) {
  }

  ~Tree() {
    if (tree_) {
      git_tree_free(tree_);
    }
  }

 private:
  git_tree* tree_;
};

} // libgit2pp
