#include "Oid.h"
#include <iostream>
#include <string.h>

using namespace std;

namespace libgit2pp {

Oid::Oid(const std::string& hex) : valid_(false) {
  if (0 != git_oid_fromstrn(&oid_, hex.c_str(), hex.size())) {
    cerr << __FILE__ << ":" << __LINE__ << " "
         << " failes to convert a hex value " << hex << endl;
  } else {
    valid_ = true;
  }
}

Oid::Oid(const Oid& b) : valid_(b.valid_) {
  if (valid_) {
    memcpy(&oid_, &b.oid_, sizeof(oid_));
  }
}

string Oid::toString() const {
  string ret;
  // An Oid hex string is at least 40 bytes.
  ret.resize(80);
  git_oid_nfmt(&ret[0], ret.size(), &oid_);
  auto pos = ret.find('\0');
  if (pos != string::npos) {
    ret.resize(pos);
  }
  return ret;
}

}
