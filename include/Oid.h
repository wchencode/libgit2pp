#pragma once

#include "git2.h"
#include <string>

namespace libgit2pp {

class Oid {
 public:
  // Create an Oid from a hex representation.
  explicit Oid(const std::string& hex);

  // Copy another Oid.
  Oid(const Oid& b);

  // Return false if the Oid is not valid.
  bool isValid() const { return valid_; }

  // Convert an Oid to its hex representation.
  std::string toString() const;

 private:
  git_oid oid_;
  bool valid_;
};

}
