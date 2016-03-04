#pragma once

namespace libgit2pp {

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
   @param avgDeletedFileNumber the average number of files to be deleted
                               in a diff.
   @param avgDirDepth the average depth of directory.
   @param finalNumberOfDirectory how many directories are when the
                                 generator completes.
   @param finalNumberOfFiles how many files are when generator completes.
  */
  DiffGenerator(
      int avgFileSize,
      double avgFileNumber,
      double avgOverlappingFileNumber,
      double avgDeletedFileNumber,
      double avgDirDepth,
      int finalNumberOfDirectory,
      int finalNumberOfFiles);

  /**
   Use this method to obtain generated diffs.

   @param addedFiles the set of files and their corresponding contents
                     in the diff.
   @param deletedFiles the set of files to be removed in the diff.
   @returns false if there is no more diffs to generate according
            to specification. If false is returned, the fields
            @param addedFiles and @param deletedFiles should be
            ignored by the caller.
  */
  bool next(
      std::unordered_map<std::string, std::string>* addedFiles,
      std::unordered_set<std::string>* deletedFiles);

  int getNumberOfFiles() const;

  int getNumberOfDirectories() const;

 private:
};

} // libgit2pp
