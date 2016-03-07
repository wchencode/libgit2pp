#include "PathTree.h"
#include "TestUtils.h"
#include <iostream>

using namespace std;

namespace libgit2pp {

PathTree::Node::Node()
    : parent(nullptr),
      maxDepth(0),
      totalSubDirs(0),
      totalFiles(0) {
}

PathTree::Node* PathTree::Node::findChild(const string& name) {
  for (auto p : children) {
    if (p->name == name) {
      return p;
    }
  }
  return nullptr;
}

PathTree::PathTree() : root_(new Node) {}

PathTree::~PathTree() {
  release(root_);
}

void PathTree::release(Node* root) {
  for (auto p : root->children) {
    release(p);
  }
  delete root;
}

const PathTree::Node* PathTree::find(const string& path) {
  auto parts = splitFilePath(path);
  if (!parts.empty()) {
    return findInternal(root_, parts, 0);
  } else {
    return root_;
  }
}

const PathTree::Node* PathTree::
findInternal(Node* root, const vector<string>& parts, int curIdx) {
  auto p = root->findChild(parts[curIdx]);
  if (p != nullptr) {
    if (parts.size() == curIdx + 1) {
      return p;
    } else {
      return findInternal(p, parts, curIdx+1);
    }
  } else {
    return nullptr;
  }
}

const PathTree::Node* PathTree::createRecursively(const string& path) {
  auto parts = splitFilePath(path);
  if (!parts.empty()) {
    return createRecursivelyInternal(root_, parts, 0).first;
  } else {
    return root_;
  }
}

pair<const PathTree::Node*, bool> PathTree::
createRecursivelyInternal(Node* root, const vector<string>& parts, int idx) {
  auto p = root->findChild(parts[idx]);
  bool needPropogate = false;

  if (p != nullptr && idx < parts.size() - 1) {
    // We are searching through existing nodes.
    int savedSubDirs = p->totalSubDirs;
    int savedFiles = p->totalFiles;

    auto ret = createRecursivelyInternal(p, parts, idx+1);
    if (ret.second) {
      if (root->maxDepth < p->maxDepth + 1) {
        root->maxDepth = p->maxDepth + 1;
        needPropogate = true;
      }
      if (savedSubDirs != p->totalSubDirs) {
        root->totalSubDirs += (p->totalSubDirs - savedSubDirs);
        needPropogate = true;
      }
      if (savedFiles != p->totalFiles) {
        root->totalFiles += (p->totalFiles - savedFiles);
        needPropogate = true;
      }
    }

    return make_pair(ret.first, needPropogate);
  } else if (p != nullptr && idx >= parts.size() - 1) {
    // The path we are searching for does not lead to a file, but
    // an intermediate directory instead.
    return make_pair(p, false);
  } else {
    // The nodes we need do not exist yet, create them.
    for (int i = idx; i < parts.size(); ++i) {
      auto n = new Node;
      n->name = parts[i];
      n->parent = p;
      n->totalFiles = 1;
      // Leaf node has a depth of 0.
      n->maxDepth = parts.size() - i - 1;
      n->totalSubDirs = n->maxDepth;
      root->children.push_back(n);
      p = n;
      if (!needPropogate) {
        needPropogate = true;
      }
    }

    if (needPropogate) {
      root->maxDepth = parts.size() - idx;
      root->totalSubDirs += (parts.size() - idx);
      ++root->totalFiles;
    }

    return make_pair(p, needPropogate);
  }
}

}  // libgit2pp
