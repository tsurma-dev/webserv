#include "../includes/webserv.hpp"

void split(const std::string& str, std::vector<std::string>& components) {
    std::string item;
    std::stringstream ss(str);
    while (std::getline(ss, item, '/')) {
        components.push_back(item);
    }
}

void insert(TrieNode* root, const std::string& path, unsigned char permissions) {
    std::vector<std::string> components;
    split(path, components);

    TrieNode* current = root;
    for (const std::string& component : components) {
        if (current->children.find(component) == current->children.end()) {
            current->children[component] = new TrieNode();
        }
        current = current->children[component];
    }
    current->permissions = permissions;
}

TrieNode* findBestMatch(TrieNode* root, const std::string& filepath) {
    std::vector<std::string> filepathComponents;
    split(filepath, filepathComponents);
    TrieNode* current = root;
    TrieNode* bestMatchNode = nullptr;

    for (const std::string& component : filepathComponents) {
        if (current->children.find(component) != current->children.end()) {
            current = current->children[component];
            bestMatchNode = current;
        } else {
            break;
        }
    }
    return bestMatchNode;
}

void deleteTrie(TrieNode* root) {
    delete root;
}

