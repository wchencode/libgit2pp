#include "DiffGenerator.h"
#include "Wrapper.h"
#include "TestUtils.h"

#include <sstream>
#include <iostream>

using namespace libgit2pp;
using namespace std;

const int avgFileSize = 128;
const int avgFileNumber = 16;
const int avgOverlappingFileNumber = 1;
const int avgDirDepth = 4;
const int topDirFanout = 20;
const int middleDirFanout = 5;
const int leafDirFanout = 10;
const int finalNumberOfFiles = 100;

const string root("/tmp/LoadTest");

main() {
  Git2 git2;
  setupRoot(root);

  // Create a bare repository.
  unique_ptr<Repository> r;
  try {
    r = make_unique<Repository>(root, true);
  } catch (const exception& ex) {
    throw runtime_error("Fails to create a new git repository");
  }

  DiffGenerator gen(
      avgFileSize,
      avgFileNumber,
      avgOverlappingFileNumber,
      avgDirDepth,
      topDirFanout,
      middleDirFanout,
      leafDirFanout,
      finalNumberOfFiles);

  for (int i = 0; gen.getNumberOfFiles() < finalNumberOfFiles; ++i) {
    unordered_map<string, string> diff;
    if (!gen.next(&diff)) {
      throw runtime_error("Fails to generate next diff");
    }

    string commitMessage;
    {
      stringstream ss;
      ss << "A testing commit #" << i;
      commitMessage = ss.str();
    }

    string id = r->commit(
        "HEAD",
        "My Name",
        "my.name@gmail.com",
        commitMessage,
        diff,
        unordered_set<string>());

    if (id.empty()) {
      throw runtime_error("Fails to create a commit");
    }

    if (i % 20 == 19) {
      cerr << "At " << i << "th commits, " << gen.getNumberOfFiles()
           << " of files created." << endl;
    }
  }
}
