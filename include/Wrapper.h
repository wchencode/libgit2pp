#pragma once

#include "git2.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>

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

  /**
   Clone a remote repository.

   By default this creates its repository and initial remote to match
   git's defaults.

   @param url the remote repository to clone
   @param localPath local directory to clone to
  */
  Repository(const std::string& url, const std::string& localPath);

  // Moving constructor.
  Repository(Repository&& b);

  ~Repository();

  /**
   Create new commit in the repository.

   @param updateRef If not empty, name of the reference that
          will be updated to point to this commit. If the reference
          is not direct, it will be resolved to a direct reference.
          Use "HEAD" to update the HEAD of the current branch and
          make it point to this commit. If the reference doesn't
          exist yet, it will be created. If it does exist, the first
          parent must be the tip of this branch.
   @param authorName
   @param authorEmail
   @param message Full message for this commit
   @param additions maps relative file paths to their corresponding
          contents. The files are either changed or added in the
          commit.
   @param deletions a list of relative paths to be deleted in the commit.

   @returns a hex representation of the commit ID just created, or
            an empty string if commit fails.
  */
  std::string commit(
      const std::string& updateRef,
      const std::string& authorName,
      const std::string& authorEmail,
      const std::string& message,
      const std::unordered_map<std::string, std::string>& additions,
      const std::unordered_set<std::string> deletions);

  /*
   Create a new tree in object database.

   @Param id If the method returns true, this is the object ID of the
   new git tree created.

   @Param source the hex representation of an object id (original tree).
   If it is not empty, the new tree will be initialized with the entries
   of the original tree.

   @Param addedFiles the relative path of files that is going to be added
   or changed and their associated object IDs.

   @Param deletedFiles the relative path of files that is going to be
   deleted.
  */
  bool createTreeUsingExistingTree(
      git_oid* id,
      const std::string& source,
      const std::unordered_map<std::string, git_oid*>& addedFiles,
      const std::unordered_set<std::string>& deletedFiles);

  /*
   Create a new tree in object database.

   @Param id If the method returns true, this is the object ID of the
   new git tree created.

   @Param commit the hex representation of a commit. If it is not empty,
   the new tree will be initialized with the entries from the tree of
   the commit. Otherwise, the new tree will be initialized with the
   entries from the tree of the HEAD.

   @Param addedFiles the relative path of files that is going to be added
   or changed and their associated object IDs.

   @Param deletedFiles the relative path of files that is going to be
   deleted.
  */
  bool createTreeUsingCommit(
      git_oid* id,
      const std::string& commit,
      const std::unordered_map<std::string, git_oid*>& addedFiles,
      const std::unordered_set<std::string>& deletedFiles);

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

  /**
   Create new commit in the repository from a list of `git_object` pointers

   @param id Pointer in which to store the OID of the newly created commit
   @param updateRef If not empty, name of the reference that
          will be updated to point to this commit. If the reference
          is not direct, it will be resolved to a direct reference.
          Use "HEAD" to update the HEAD of the current branch and
          make it point to this commit. If the reference doesn't
          exist yet, it will be created. If it does exist, the first
          parent must be the tip of this branch.
   @param authorName
   @param authorEmail
   @param message Full message for this commit
   @param tree An instance of a `git_tree` object that will
    be used as the tree for the commit. This tree object must
    also be owned by the given `repo`.
   @param parentCount Number of parents for this commit
   @param parents Array of `parent_count` pointers to `git_commit`
    objects that will be used as the parents for this commit. This
    array may be NULL if `parent_count` is 0 (root commit). All the
    given commits must be owned by the `repo`.
  */
  bool commit(
      git_oid* id,
      const std::string& updateRef,
      const std::string& authorName,
      const std::string& authorEmail,
      const std::string& message,
      const git_tree *tree,
      size_t parentCount,
      const git_commit *parents[]);

  // Get git_reference object for HEAD.
  git_reference* getHead();

  // Get object database of this repository.
  git_odb* getOdb();

  /**
   Lookup a commit object from a repository.

   @param id identity of the commit to locate. If the object is
          an annotated tag it will be peeled back to the commit.
  */
  git_commit* getCommit(const git_oid* id);

  // Get the information for a particular remote
  // @param name the remote's name
  git_remote* getRemote(const std::string& name);

  git_repository* get() { return repo_; }

  // Returns git_repository pointer. The caller needs to
  // release it later.
  git_repository* release();

 private:
  git_repository* repo_;

  bool createTreeUsingGitTree(
      git_oid* id,
      std::unique_ptr<git_tree> tree,
      const std::unordered_map<std::string, git_oid*>& addedFiles,
      const std::unordered_set<std::string>& deletedFiles);
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

} // libgit2pp

namespace std {

template <> struct default_delete<git_tree> {
  void operator()(git_tree* tree) const {
    if (tree) {
      git_tree_free(tree);
    }
  }
};

template <> struct default_delete<git_reference> {
  void operator()(git_reference* ref) const {
    if (ref) {
      git_reference_free(ref);
    }
  }
};

template <> struct default_delete<git_odb> {
  void operator()(git_odb* odb) const {
    if (odb) {
      git_odb_free(odb);
    }
  }
};

template <> struct default_delete<git_commit> {
  void operator()(git_commit* commit) const {
    if (commit) {
      git_commit_free(commit);
    }
  }
};

template <> struct default_delete<git_remote> {
  void operator()(git_remote* remote) const {
    if (remote) {
      git_remote_free(remote);
    }
  }
};

template <> struct default_delete<git_treebuilder> {
  void operator()(git_treebuilder* b) const {
    if (b) {
      git_treebuilder_free(b);
    }
  }
};

} // std
