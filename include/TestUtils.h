#pragma once

#include <string>

namespace libgit2pp {

// Recreate directory trees at @param root.
void setupRoot(const std::string& root);

// Write @param data to file path @param path.
void writeToFile(const std::string& path, const std::string& data);

}
