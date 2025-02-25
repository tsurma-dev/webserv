#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "ParseUtils.hpp"

// Forward declaration of ParsedServer (instead of Server)
class ParsedServer;

class Location {
private:
    std::string path;
    std::string root;
    std::string index;
    bool autoindex;
    std::vector<std::string> methods;
    bool is_cgi;
    std::string cgi_extension;
    std::string cgi_pass;
    std::map<int, std::string> error_pages;
    size_t client_max_body_size;
    std::map<int, std::string> return_url;

public:
    Location();
    // Constructor now takes a ParsedServer instead of Server
    Location(const ParsedServer &server);
    Location(const Location &location);
    Location &operator=(const Location &location);
    ~Location();

    const std::string &getPath() const;
    const std::string &getRoot() const;
    const std::string &getIndex() const;
    bool getAutoIndex() const;
    const std::vector<std::string> &getMethods() const;
    bool isCgi() const;
    const std::string &getCgiExtension() const;
    const std::string &getCgiPass() const;
    const std::map<int, std::string> &getErrorPages() const;
    size_t getClientMaxBodySize() const;
    const std::map<int, std::string> &getReturnUrl() const;

    // Setters
    void setPath(const std::string &path);
    void setRoot(const std::string &root);
    void setIndex(const std::string &index);
    void setAutoIndex(bool autoindex);
    void setMethods(const std::vector<std::string> &methods);
    void setIsCgi(bool is_cgi);
    void setCgiExtension(const std::string &cgi_extension);
    void setCgiPass(const std::string &cgi_pass);
    void setErrorPages(const std::map<int, std::string> &error_pages);
    void setClientMaxBodySize(size_t client_max_body_size);
    void setReturnUrl(const std::map<int, std::string> &return_url);

    void printLocation() const;
};

#endif
