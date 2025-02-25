#ifndef PARSEDSERVER_HPP
#define PARSEDSERVER_HPP

#include "Location.hpp"

class Config;

class ParsedServer {
private:
    std::string server_name;
    std::vector<std::string> host;
    std::string dir_list;
    std::string root;
    std::vector<int> ports;
    std::map<std::string, Location> locations;
    std::map<int, std::string> errorPages;
    std::string index;
    bool autoindex;
    size_t client_max_body_size;
    std::vector<std::string> allowed_methods;

public:
    ParsedServer(const Config &config);
    ParsedServer(const ParsedServer &other);
    ParsedServer &operator=(const ParsedServer &other);
    ~ParsedServer();

    const std::string &getServerName() const;
    const std::vector<std::string> &getHost() const;
    const std::string &getDirList() const;
    const std::string &getRoot() const;
    const std::vector<int> &getPorts() const;
    const std::map<std::string, Location> &getLocations() const;
    const std::map<int, std::string> getErrorPages() const;
    const std::string &getIndex() const;
    bool getAutoIndex() const;
    size_t getMaxBodySize() const;
    const std::vector<std::string> &getAllowedMethods() const;

    void setServerName(std::string name);
    void setHost(std::vector<std::string> host);
    void setDirList(std::string dir_list);
    void setRoot(std::string root);
    void setPort(std::vector<int> ports);
    void setLocations(std::map<std::string, Location> locations);
    void setErrorPages(std::map<int, std::string> error_pages);
    void setIndex(std::string index);
    void setAutoIndex(bool autoindex);
    void setMaxBodySize(size_t client_max_body_size);
    void setAllowedMethods(std::vector<std::string> allowed_methods);

    void addLocation(const std::string &path, const Location &location);

    void printServer() const;
};

#endif
