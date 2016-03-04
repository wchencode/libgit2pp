#pragma once

#include <string>
#include <vector>

namespace libgit2pp {

// Recreate directory trees at @param root.
void setupRoot(const std::string& root);

// Write @param data to file path @param path.
void writeToFile(const std::string& path, const std::string& data);

// Split file path @param path into multiple components. Example:
// If input is "a/b/c/d", the result is a list {"a", "b", "c", "d"}.
// The input path should not have leading '/'.
std::vector<std::string> splitFilePath(const std::string& path);

}
