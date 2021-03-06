#include "Wrapper.h"
#include "TestUtils.h"

#include <stdexcept>
#include <iostream>
#include <vector>
#include <tuple>
#include <sstream>
#include <map>

#include <unistd.h>

using namespace std;

namespace libgit2pp {

Repository::Repository(git_repository* repo) : repo_(repo) {
}

Repository::Repository(const string& path) {
  if (0 != git_repository_open(&repo_, path.c_str())) {
    throw runtime_error("Fails to open a repository");
  }
}

Repository::Repository(const string& path, bool isBare) {
  if (0 != git_repository_init(&repo_, path.c_str(), isBare)) {
    throw runtime_error("Fails to create a repository");
  }
}

Repository::Repository(const std::string& url, const std::string& localPath)
    : repo_(nullptr) {
  if (0 != git_clone(&repo_, url.c_str(), localPath.c_str(), nullptr)) {
    throw runtime_error("Fails to clone a git repository");
  }
}

Repository::Repository(Repository&& b) {
  std::swap(repo_, b.repo_);
}

Repository::~Repository() {
  if (repo_) {
    git_repository_free(repo_);
  }
}

string Repository::commit(
    const string& updateRef,
    const string& authorName,
    const string& authorEmail,
    const string& message,
    const unordered_map<string, string>& additions,
    const unordered_set<string> deletions) {
  // Obtain a tmpfile to write file contents to.
  string tmpfile;
  {
    stringstream ss;
    ss << "/tmp/git_commits/" << (uint64_t)pthread_self();
    auto tmpdir = ss.str();
    string cmd = string("mkdir -p ") + tmpdir;
    system(cmd.c_str());
    tmpfile = tmpdir + "/txt";
  }

  // Create blob objects.
  vector<git_oid> oids(additions.size());
  unordered_map<string, git_oid*> addedFiles;
  {
    int idx = 0;
    for (auto& p : additions) {
      writeToFile(tmpfile, p.second);
      addedFiles[p.first] = &oids[idx];
      if (!createBlobFromDisk(tmpfile, &oids[idx++])) {
        throw runtime_error("Fails to create an object in git");
      }
      unlink(tmpfile.c_str());
    }
  }

  git_oid id;
  if (createTreeUsingCommit(&id, "", addedFiles, deletions)) {
    unique_ptr<git_commit> c(getHeadCommit());
    unique_ptr<git_tree> tree(getTree(&id));

    if (c.get() != nullptr) {
      // There are commits before.
      const git_commit* parents[] = { c.get() };
      if (!commit(&id, updateRef, authorName, authorEmail, message,
          tree.get(), 1, parents)) {
        return string();
      }
    } else {
      // This is the first commit in the repository.
      if (!commit(&id, updateRef, authorName, authorEmail, message,
          tree.get(), 0, nullptr)) {
        return string();
      }
    }
  }

  string ret;
  ret.resize(GIT_OID_HEXSZ);
  git_oid_nfmt(const_cast<char*>(ret.data()), ret.size(), &id);
  return ret;
}

bool Repository::createTreeUsingExistingTree(
    git_oid* id,
    const string& source,
    const unordered_map<std::string, git_oid*>& addedFiles,
    const unordered_set<std::string>& deletedFiles) {
  git_tree* tree = nullptr;
  if (!source.empty()) {
    git_oid treeId;
    if (0 != git_oid_fromstr(&treeId, source.c_str())) {
      cerr << "Fails to convert a hex string into object ID" << endl;
      return false;
    }

    tree = getTree(&treeId);
    if (tree == nullptr) {
      return false;
    }
  }

  return createTreeUsingGitTree(
      id, unique_ptr<git_tree>(tree), addedFiles, deletedFiles);
}

git_commit* Repository::getHeadCommit() {
  const git_oid* target = nullptr;
  git_oid commitId;

  // Get the head.
  unique_ptr<git_reference> gr(getHead());
  if (gr.get() != nullptr) {
    target = git_reference_target(gr.get());
  }

  // If there is no HEAD yet, it is the first commit in the repository.
  if (!target) {
    return nullptr;
  }

  // Verify target's type.
  unique_ptr<git_odb> odb(getOdb());
  size_t len = 0;
  git_otype type;
  if (0 != git_odb_read_header(&len, &type, odb.get(), target)) {
    throw runtime_error("Fails to read object from ODB");
  }
  if (type != GIT_OBJ_COMMIT) {
    throw runtime_error("Expect a commit object");
  }

  return getCommit(target);
}

bool Repository::createTreeUsingCommit(
    git_oid* id,
    const string& commit,
    const unordered_map<std::string, git_oid*>& addedFiles,
    const unordered_set<std::string>& deletedFiles) {
  const git_oid* target = nullptr;
  unique_ptr<git_commit> c;

  if (!commit.empty()) {
    // Get the object ID for the commit.
    git_oid commitId;
    if (0 != git_oid_fromstr(&commitId, commit.c_str())) {
      cerr << "Fails to convert a hex string into object ID" << endl;
      return false;
    } else {
      c.reset(getCommit(&commitId));
    }
    if (c.get() == nullptr) {
      throw runtime_error("Fails to get commit");
    }
  } else {
    c.reset(getHeadCommit());
  }

  git_tree* tmpTree = nullptr;
  if (c.get() != nullptr) {
    if (0 != git_commit_tree(&tmpTree, c.get())) {
      throw runtime_error("Fails to get existing tree");
    }
  }

  return createTreeUsingGitTree(
      id, unique_ptr<git_tree>(tmpTree), addedFiles, deletedFiles);
}

git_treebuilder* Repository::createTreeBuilder(const git_tree* source) {
  git_treebuilder* out = nullptr;
  if (0 == git_treebuilder_new(&out, repo_, source)) {
    return out;
  } else {
    return nullptr;
  }
}

git_tree* Repository::getTree(const git_oid* id) {
  git_tree* out = nullptr;
  if (0 == git_tree_lookup(&out, repo_, id)) {
    return out;
  } else {
    return nullptr;
  }
}

bool Repository::createBlobFromDisk(const std::string& path, git_oid* id) {
  return (0 == git_blob_create_fromdisk(id, repo_, path.c_str()));
}

bool Repository::commit(
    git_oid* id,
    const string& updateRef,
    const string& authorName,
    const string& authorEmail,
    const string& message,
    const git_tree *tree,
    size_t parentCount,
    const git_commit *parents[]) {
  git_signature* sig = nullptr;
  if (0 != git_signature_now(&sig, authorName.c_str(), authorEmail.c_str())) {
    return false;
  }
  int ret = git_commit_create(
                id,
                repo_,
                updateRef.empty() ? nullptr : updateRef.c_str(),
                sig, /*const gitsignature* author*/
                sig, /*const gitsignature* committer*/
                nullptr, /*const char* message_encoding*/
                message.c_str(),
                tree,
                parentCount,
                parents);

  git_signature_free(sig);
  return (ret == 0);
}

git_reference* Repository::getHead() {
  git_reference* out = nullptr;
  if (0 == git_repository_head(&out, repo_)) {
    return out;
  } else {
    return nullptr;
  }
}

git_odb* Repository::getOdb() {
  git_odb* out = nullptr;
  if (0 == git_repository_odb(&out, repo_)) {
    return out;
  } else {
    return nullptr;
  }
}

git_commit* Repository::getCommit(const git_oid* id) {
  git_commit* commit = nullptr;
  if (0 == git_commit_lookup(&commit, repo_, id)) {
    return commit;
  } else {
    return nullptr;
  }
}

git_remote* Repository::getRemote(const string& name) {
  git_remote* out = nullptr;
  if (0 == git_remote_lookup(&out, repo_, name.c_str())) {
    return out;
  } else {
    return nullptr;
  }
}

git_repository* Repository::release() {
  auto ret = repo_;
  repo_ = nullptr;
  return ret;
}

string joinFilePath(const vector<string>& parts, int start, int end) {
  stringstream ss;
  for (int i = start; i < end; ++i) {
    ss << parts[i];
    if (i < end - 1) {
      ss << "/";
    } else {
      break;
    }
  }
  return ss.str();
}

git_treebuilder* getTreeBuilder(
    unordered_map<string, unique_ptr<git_treebuilder>>* builderMap,
    git_repository* repo,
    const string& path,
    const git_tree* ptree) {
  auto it = builderMap->find(path);
  if (it != builderMap->end()) {
    return it->second.get();
  }
  git_treebuilder* out = nullptr;
  if (0 != git_treebuilder_new(&out, repo, ptree)) {
    throw runtime_error("Fails to create a new treebuilder");
  }
  (*builderMap)[path] = unique_ptr<git_treebuilder>(out);
  return out;
}

bool Repository::createTreeUsingGitTree(
    git_oid* idOut,
    unique_ptr<git_tree> tree,
    const unordered_map<string, git_oid*>& addedFiles,
    const unordered_set<string>& deletedFiles) {
  // Collect all files that is going to be updated.
  vector<string> affectedFiles;
  for (auto& p : addedFiles) {
    affectedFiles.push_back(p.first);
  }
  for (auto& s : deletedFiles) {
    affectedFiles.push_back(s);
  }

  // Maps a prefix to a pair. A prefix is the relative path of a directory.
  // For a relative file path of "a/b/c/d", prefixes are "a", "a/b",
  // "a/b/c", base file name is "d".
  //
  // The map has all prefixes for a given file. The base name, however,
  // only appears in the value field of the longest prefix.
  //
  // The first field of the pair are a list of base file names that has
  // been changed in the prefix directory.
  //
  // The second field is the corresponding git_tree's position in
  // @param queue, or -1 if the tree is a new one. Initially all second
  // fields are -1.
  map<string, pair<vector<string>, int>> changeTreeMap;
  for (auto& s : affectedFiles) {
    vector<string> parts = splitFilePath(s);
    if (parts.empty()) {
      cerr << "An empty string cannot be a valid path name" << endl;
      return false;
    }

    string name = parts.back();
    parts.pop_back();

    for (int i = parts.size(); i >= 0; --i) {
      // Allow empty string to be returned.
      auto prefix = joinFilePath(parts, 0, i);
      if (i == parts.size()) {
        changeTreeMap[prefix].first.push_back(name);
        changeTreeMap[prefix].second = -1;
      } else if (changeTreeMap.count(prefix) == 0) {
        changeTreeMap[prefix] = make_pair(vector<string>(), -1);
      } else {
        break;
      }
    }
  }

  // For the special key "", we know its git_tree's position in
  // @param queue is 0.
  {
    auto it = changeTreeMap.find("");
    if (it != changeTreeMap.end()) {
      it->second.second = 0;
    }
  }

  // Defines the fields used in the tuple below.
  enum {
    // The position in the queue that holds the parent entry.
    parent = 0,
    // The (relative) path of the git_tree.
    rel_path,
    // Pointer to the corresponding git_tree.
    tree_ptr,
  };

  // A queue (actually a stack) that helps WFS search of sub-trees.
  // Each element of the queue is a tuple whose elements is defined
  // in the enum above.
  vector<tuple<int, string, unique_ptr<git_tree>>> queue;
  queue.push_back(make_tuple(-1, string(), std::move(tree)));

  // A width-first traverse to find all sub-trees that are affected
  // by the update.
  for (int pos = 0; pos < queue.size(); ++pos) {
    git_tree* ptree = std::get<tree_ptr>(queue[pos]).get();
    int entrycount = 0;
    if (ptree) {
      entrycount = git_tree_entrycount(ptree);
    }

    // Find sub-trees of current node.
    for (int i = 0; i < entrycount; ++i) {
      auto entry = git_tree_entry_byindex(ptree, i);
      if (entry == nullptr) {
        cerr << "Entry should not be null" << endl;
        return false;
      }

      git_otype otype = git_tree_entry_type(entry);
      if (otype == GIT_OBJ_TREE) {
        // Figure out the correct key for @param changeTreeMap.
        const char* base = git_tree_entry_name(entry);
        const auto& relPath = std::get<rel_path>(queue[pos]);
        string name;
        if (!relPath.empty()) {
          name = relPath + "/" + base;
        } else {
          name = base;
        }

        auto it = changeTreeMap.find(name);

        // We only interested in affected paths.
        if (it != changeTreeMap.end()) {
          const git_oid* id = git_tree_entry_id(entry);
          git_tree* child = nullptr;
          if (0 != git_tree_lookup(&child, repo_, id)) {
            cerr << "Fails to lookup a tree id" << endl;
            return false;
          } else {
            it->second.second = queue.size();
            queue.emplace_back(pos, name, unique_ptr<git_tree>(child));
          }
        }
      }
    }
  }

  // Find all entries in the changeTreeMap that has not be touched yet.
  for (auto& p : changeTreeMap) {
    if (p.second.second < 0) {
      string prefix;
      auto pos = p.first.rfind('/');
      if (pos != string::npos) {
        prefix = string(p.first, 0, pos);
      }
      auto it = changeTreeMap.find(prefix);
      if (it == changeTreeMap.end()) {
        throw runtime_error("Fails to find prefix " + prefix);
      }
      p.second.second = queue.size();
      queue.emplace_back(it->second.second, p.first, nullptr);
    }
  }

  // Work backward to create new trees without dependency.
  unordered_map<string, unique_ptr<git_treebuilder>> builderMap;
  for (int i = queue.size() - 1; i >= 0; --i) {
    const string& name = std::get<rel_path>(queue[i]);
    bool removeCurrentTree = false;

    // Object ID of current tree, to be filled later.
    git_oid id;

    // Update current tree, inserts or remove files as requested.
    {
      // Test the difference between addition and deletion. If deletions
      // outnumber additions, there is a chance that the directory (tree)
      // should be removed.
      int diff = 0;

      // Obtain current tree's git_treebuilder.
      const git_tree* ptree = std::get<tree_ptr>(queue[i]).get();
      git_treebuilder* b = getTreeBuilder(&builderMap, repo_, name, ptree);

      // Find out if there is any changes (files updates) for current tree.
      auto it = changeTreeMap.find(name);
      if (it != changeTreeMap.end()) {
        for (auto& s : it->second.first) {
          auto path = name.empty() ? s : name + "/" + s;
          auto found = addedFiles.find(path);
          if (found != addedFiles.end()) {
            ++diff;
            const git_tree_entry* out = nullptr;
            auto mode = (git_filemode_t)0100644;
            git_treebuilder_insert(&out, b, s.c_str(), found->second, mode);
          } else if (deletedFiles.count(path) > 0) {
            --diff;
            git_treebuilder_remove(b, s.c_str());
          }
        }
      }

      // Test if current tree should be removed.
      // TODO: verify that git_treebuilder_entrycount is working as intended.
      if (diff < 0 && git_treebuilder_entrycount(b) == 0) {
        removeCurrentTree = true;
      }

      // Finalize and create a new tree.
      if (!removeCurrentTree) {
        if (0 != git_treebuilder_write(&id, b)) {
          throw runtime_error("Fails to create a new tree object");
        }
      }
    }

    // Update parent.
    if (i > 0) {
      // Obtain parent's relative path.
      string prefix, base;
      auto pos = name.rfind('/');
      if (pos != string::npos) {
        prefix = string(name, 0, pos);
        base = name.substr(pos+1);
      } else {
        base = name;
      }

      // Obtain the treebuilder for the parent.
      auto parentIdx = std::get<parent>(queue[i]);
      const git_tree* ptree = nullptr;

      if (parentIdx >= 0) {
        ptree = std::get<tree_ptr>(queue[parentIdx]).get();
      }

      git_treebuilder* b = getTreeBuilder(&builderMap, repo_, prefix, ptree);

      // Update the treebuilder.
      if (!removeCurrentTree) {
        const git_tree_entry* out = nullptr;
        auto mode = (git_filemode_t)0040000;
        git_treebuilder_insert(&out, b, base.c_str(), &id, mode);
      } else {
        git_treebuilder_remove(b, base.c_str());
      }
    } else {
      // This is the last iteration in the loop.
      if (removeCurrentTree) {
        // The repo becomes empty. No tree object ID shall be returned.
        return false;
      } else {
        git_oid_cpy(idOut, &id);
      }
    }
  }

  return true;
}

}
