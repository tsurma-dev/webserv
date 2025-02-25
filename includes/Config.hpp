#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>

#include "ParsedServer.hpp"

class ParsedServer;

class Config {
private:
    std::vector<ParsedServer> servers;
    std::map<int, std::string> errorPages;
    std::string configPath;
    size_t maxBodySize;
    bool globalAutoIndex;
    std::vector<std::string> globalAllowedMethods;
    std::string globalDirList;

public:
    Config(const std::string &configPath);
    Config(const Config &config);
    Config &operator=(const Config &config);
    ~Config();

    size_t getMaxBodySize() const;
    const std::map<int, std::string> &getErrorPages() const;
    bool getGlobalAutoIndex() const;
    const std::vector<std::string> &getGlobalAllowedMethods() const;
    const std::vector<ParsedServer> getServers() const;

    void parseConfigFile(const std::string &configPath);
    void parseServerBlock(std::ifstream &file);

    void parseLocationBlock(std::ifstream &file, ParsedServer &server, const std::string &openLine);

    void printConfig() const;
};

std::string getFirstToken(const std::string &line);

#endif
