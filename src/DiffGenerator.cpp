#include "DiffGenerator.h"
#include "PathTree.h"

#include <cmath>

using namespace std;

namespace libgit2pp {

// Deviation for log normal distribution.
const double lognormalDev = 0.25;

DiffGenerator::DiffGenerator(
    int avgFileSize,
    int avgFileNumber,
    int avgOverlappingFileNumber,
    int avgDirDepth,
    int topDirFanout,
    int middleDirFanout,
    int leafDirFanout,
    int finalNumberOfFiles)
        : avgFileSize_(avgFileSize),
          avgFileNumber_(avgFileNumber),
          avgOverlappingFileNumber_(avgOverlappingFileNumber),
          avgDirDepth_(avgDirDepth),
          topDirFanout_(topDirFanout),
          middleDirFanout_(middleDirFanout),
          leafDirFanout_(leafDirFanout),
          finalNumberOfFiles_(finalNumberOfFiles),
          mt_(random_device()()),
          tree_(new PathTree) {
}

DiffGenerator::~DiffGenerator() {
}

bool DiffGenerator::next(unordered_map<string, string>* addedFiles) {
  auto root = tree_->find("");
  if (root->totalFiles > finalNumberOfFiles_) {
    return false;
  }

  // Figure out how many files are in the diff.
  int numFiles = 0;
  {
    lognormal_distribution<> d(log(avgFileNumber_ - 1), lognormalDev);
    numFiles = std::round(d(mt_)) + 1;
  }

  int overlappingFiles = 0;
  {
    lognormal_distribution<> d(log(avgOverlappingFileNumber_), lognormalDev);
    overlappingFiles = std::round(d(mt_));
    if (overlappingFiles > numFiles) {
      overlappingFiles = numFiles;
    }
  }

  for (int i = 0; i < numFiles; ++i) {
    // Pick the depth of the generated path.
    int dirDepth = 0;
    {
      // The depth should at least be 3. In that case we have one top
      // level dir, one leaf level dir, and a file (which counts as a level).
      lognormal_distribution<> d(log(avgDirDepth_ - 3), lognormalDev);
      dirDepth = std::round(d(mt_)) + 3;
    }

    string path;
    bool createDir = false;

    // Pick top level directory.
    auto p = root;
    if (root->children.size() < topDirFanout_) {
      // Create new path.
      path = path + genFileName() + "/";
      createDir = true;
    } else {
      // Use existing path.
      uniform_int_distribution<> dis(0, p->children.size() - 1);
      auto idx = dis(mt_);
      p = p->children[idx];
      path = path + p->name + "/";
    }

    // Pick middle level directories.
    for (int i = 0; i < dirDepth - 3; ++i) {
      if (!createDir) {
        lognormal_distribution<> d(log(middleDirFanout_), lognormalDev);
        if (p->children.empty() || p->children.size() < std::round(d(mt_))) {
          // Need to create a new directory.
          createDir = true;
        } else {
          // Use existing path.
          uniform_int_distribution<> dis(0, p->children.size() - 1);
          auto idx = dis(mt_);
          p = p->children[idx];
          path = path + p->name + "/";
        }
      }
      if (createDir) {
        // Create new path.
        path = path + genFileName() + "/";
      }
    }

    // Pick leaf level directory.
    if (!createDir) {
      lognormal_distribution<> d(log(leafDirFanout_), lognormalDev);
      if (p->children.empty() || p->children.size() < std::round(d(mt_))) {
        // Need to create a new directory.
        createDir = true;
      } else {
        // Use existing path.
        uniform_int_distribution<> dis(0, p->children.size() - 1);
        auto idx = dis(mt_);
        p = p->children[idx];
        path = path + p->name + "/";
      }
    }
    if (createDir) {
      // Create new path.
      path = path + genFileName() + "/";
    }

    // Pick file name.
    if (!createDir && !p->children.empty() && i < overlappingFiles) {
      // Use existing path.
      uniform_int_distribution<> dis(0, p->children.size() - 1);
      auto idx = dis(mt_);
      p = p->children[idx];
      path = path + p->name;
    } else {
      createDir = true;
      path = path + genFileName();
      tree_->createRecursively(path);
    }

    (*addedFiles)[path] = genFileData();
  } // For loop.

  return true;
}

int DiffGenerator::getNumberOfFiles() {
  auto node = tree_->find("");
  return node->totalFiles;
}

int DiffGenerator::getNumberOfTopLevelDirectories() {
  auto node = tree_->find("");
  return node->children.size();
}

int DiffGenerator::getNumberOfTotalDirectories() {
  auto node = tree_->find("");
  return node->totalSubDirs;
}

string DiffGenerator::genFileName() {
  // Average file name length is 7.
  lognormal_distribution<> d(log(7), lognormalDev);
  int num = std::round(d(mt_));

  string ret;
  ret.resize(num);
  uniform_int_distribution<> dis(0, 25);
  for (int i = 0; i < num; ++i) {
    ret[i] = 'a' + (char)dis(mt_);
  }
  return ret;
}

string DiffGenerator::genFileData() {
  lognormal_distribution<> d(log(avgFileSize_), lognormalDev);
  int num = std::round(d(mt_));

  string ret;
  ret.resize(num);
  uniform_int_distribution<> dis(0, 35);
  for (int i = 0; i < num; ++i) {
    int val = dis(mt_);
    if (val < 26) {
      ret[i] = 'A' + (char)val;
    } else {
      ret[i] = '0' + (char)(val - 26);
    }
  }
  return ret;
}

} // libgit2pp
