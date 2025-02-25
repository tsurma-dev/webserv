#pragma once

#include "webserv.hpp"

struct TrieNode {
  std::map<std::string, TrieNode*> children;
  unsigned char permissions;

  TrieNode() : permissions(0) {}

  ~TrieNode() {
    for (std::map<const std::string, TrieNode*>::iterator it = children.begin(); it != children.end(); ++it) {
      delete (*it).second;
    }
  }
};
