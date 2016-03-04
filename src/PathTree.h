#pragma once

#include <string>
#include <vector>
#include <map>

namespace libgit2pp {

/**
 A tree that mirrors git_tree path. This class is used by
 DiffGenerator to generate diffs on a repository.
*/
class PathTree {
 public:
  // Represent a path (directory) in the path tree.
  struct Node {
    // Name of the directory (not the full name from the root).
    std::string name;
    Node* parent;
    std::vector<Node*> children;
    // Max depth from this node.
    int maxDepth;
    // Total number of subdirectories seen from this node.
    int totalSubDirs;
    // Total number of files in the directory (including files
    // in sub-directories).
    int totalFiles;

    Node();
    Node* findChild(const std::string& name);
  };

  PathTree();

  ~PathTree();

  // Find the Node corresponding to the the specified @param path.
  const Node* find(const std::string& path);

  // Create a Node for the path specified. Returns the newly created
  // node, or the existing node corresponding to the path.
  const Node* createRecursively(const std::string& path);

 private:
  Node* root_;

  const Node* findInternal(
      Node* root,
      const std::vector<std::string>& parts,
      int curIdx);

  // The first field is a pointer to the node that corresponding to the path.
  // The second field is true if some parameters (total number of files, max
  // depths, etc) have changed in the root's child.
  std::pair<const Node*, bool> createRecursivelyInternal(
      Node* root,
      const std::vector<std::string>& parts,
      int idx);

  void release(Node* root);
};

} // libgit2pp
