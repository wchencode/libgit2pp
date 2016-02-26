#include "TestUtils.h"
#include <sstream>
#include <stdexcept>

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

namespace libgit2pp {

void setupRoot(const string& root) {
  // Remove leftover from previous runs.
  {
    stringstream ss;
    ss << "rm -rf " << root;
    system(ss.str().c_str());
  }

  // Create new directory.
  {
    stringstream ss;
    ss << "mkdir -p " << root;
    system(ss.str().c_str());
  }
}

void writeToFile(const string& path, const string& data) {
  int fd = open(path.c_str(), O_CREAT|O_WRONLY, 0644);
  if (fd < 0) {
    throw runtime_error("Fails to create a tmp file");
  }
  write(fd, data.c_str(), data.size());
  close(fd);
}

}
