#pragma once

#include <random>
#include <unordered_map>
#include <memory>
#include <string>

namespace libgit2pp {

class PathTree;

// This class generates diffs to be committed.
class DiffGenerator {
 public:
  /**
   Specify how to generate diffs.

   @param avgFileSize the average size of each file in the diff.
   @param avgFileNumber the average number of files in each diff.
   @param avgOverlappingFileNumber the average number of files that already
                             exist in the repository. This number includes
                             files that to be deleted and files that to
                             be updated in the diff.
   @param avgDirDepth the average depth of directory.
   @param topDirFanout how many top level directories when the generator
                       completes.
   @param middleDirFanout how many middle level directories (non-top and
                          non-leaf directories) when the generator completes.
   @param leafDirFanout how many leaf level directories (directories without
                        sub directories) when the generator completes.
   @param finalNumberOfFiles how many files are when generator completes.
  */
  DiffGenerator(
      int avgFileSize,
      int avgFileNumber,
      int avgOverlappingFileNumber,
      int avgDirDepth,
      int topDirFanout,
      int middleDirFanout,
      int leafDirFanout,
      int finalNumberOfFiles);

  ~DiffGenerator();

  /**
   Use this method to obtain generated diffs.

   @param addedFiles the set of files and their corresponding contents
                     in the diff.
   @returns false if there is no more diffs to generate according
            to specification. If false is returned, the fields
            @param addedFiles and @param deletedFiles should be
            ignored by the caller.
  */
  bool next(std::unordered_map<std::string, std::string>* addedFiles);

  int getNumberOfFiles();

  int getNumberOfTopLevelDirectories();

  int getNumberOfTotalDirectories();

 private:
  const int avgFileSize_;
  const int avgFileNumber_;
  const int avgOverlappingFileNumber_;
  const int avgDirDepth_;
  const int topDirFanout_;
  const int middleDirFanout_;
  const int leafDirFanout_;
  const int finalNumberOfFiles_;

  std::mt19937 mt_;

  std::unique_ptr<PathTree> tree_;

  // Create file or directory name.
  std::string genFileName();

  // Create text data.
  std::string genFileData();
};


} // libgit2pp
