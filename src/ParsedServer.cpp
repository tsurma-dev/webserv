#include "../includes/ParsedServer.hpp"
#include "../includes/Config.hpp"

// Constructor
ParsedServer::ParsedServer(const Config &config)
    : dir_list(""),
      autoindex(config.getGlobalAutoIndex()),
      client_max_body_size(config.getMaxBodySize()),
      allowed_methods(config.getGlobalAllowedMethods())
{
    errorPages = config.getErrorPages();
}

// Copy Constructor
ParsedServer::ParsedServer(const ParsedServer &other) {
    *this = other;
}

// Assignment Operator
ParsedServer &ParsedServer::operator=(const ParsedServer &other) {
    if (this != &other) {
        server_name         = other.server_name;
        host                = other.host;
        root                = other.root;
        ports               = other.ports;
        locations           = other.locations;
        errorPages          = other.errorPages;
        index               = other.index;
        autoindex           = other.autoindex;
        client_max_body_size= other.client_max_body_size;
        allowed_methods     = other.allowed_methods;
        dir_list            = other.dir_list;
    }
    return *this;
}

// Destructor
ParsedServer::~ParsedServer() {}

// Getters
const std::string &ParsedServer::getServerName() const {
    return server_name;
}

const std::vector<std::string> &ParsedServer::getHost() const {
    return host;
}

const std::string &ParsedServer::getDirList() const {
    return dir_list;
}

const std::string &ParsedServer::getRoot() const {
    return root;
}

const std::vector<int> &ParsedServer::getPorts() const {
    return ports;
}

const std::map<std::string, Location> &ParsedServer::getLocations() const {
    return locations;
}

const std::map<int, std::string> ParsedServer::getErrorPages() const {
    return errorPages;
}

const std::string &ParsedServer::getIndex() const {
    return index;
}

bool ParsedServer::getAutoIndex() const {
    return autoindex;
}

size_t ParsedServer::getMaxBodySize() const {
    return client_max_body_size;
}

const std::vector<std::string> &ParsedServer::getAllowedMethods() const {
    return allowed_methods;
}

// Setters
void ParsedServer::setServerName(std::string name) {
    server_name = name;
}

void ParsedServer::setHost(std::vector<std::string> host) {
    this->host = host;
}

void ParsedServer::setDirList(std::string dir_list) {
    this->dir_list = dir_list;
}

void ParsedServer::setRoot(std::string root) {
    this->root = root;
}

void ParsedServer::setPort(std::vector<int> ports) {
    this->ports = ports;
}

void ParsedServer::setLocations(std::map<std::string, Location> locations) {
    this->locations = locations;
}

void ParsedServer::setErrorPages(std::map<int, std::string> error_pages) {
    // Merging new ones with existing ones if needed
    this->errorPages.insert(error_pages.begin(), error_pages.end());
}

void ParsedServer::setIndex(std::string index) {
    this->index = index;
}

void ParsedServer::setAutoIndex(bool autoindex) {
    this->autoindex = autoindex;
}

void ParsedServer::setMaxBodySize(size_t client_max_body_size) {
    this->client_max_body_size = client_max_body_size;
}

void ParsedServer::setAllowedMethods(std::vector<std::string> allowed_methods) {
    this->allowed_methods = allowed_methods;
}

void ParsedServer::addLocation(const std::string &path, const Location &location) {
    locations[path] = location;
}

// Print function for debugging
void ParsedServer::printServer() const {
    std::cout << "Server Name: " << server_name << std::endl;

    std::cout << "Host(s): ";
    for (size_t i = 0; i < host.size(); ++i) {
        std::cout << host[i] << (i < host.size() - 1 ? ", " : "");
    }
    std::cout << std::endl;

    std::cout << "Directory Listing: " << dir_list << std::endl;
    std::cout << "Root: " << root << std::endl;

    std::cout << "Ports: ";
    for (size_t i = 0; i < ports.size(); ++i) {
        std::cout << ports[i] << (i < ports.size() - 1 ? ", " : "");
    }
    std::cout << std::endl;

    std::cout << "Index: " << index << std::endl;
    std::cout << "Autoindex: " << (autoindex ? "on" : "off") << std::endl;

    std::cout << "Allowed Methods: ";
    for (size_t i = 0; i < allowed_methods.size(); ++i) {
        std::cout << allowed_methods[i] << (i < allowed_methods.size() - 1 ? ", " : "");
    }
    std::cout << std::endl;

    std::cout << "Error Pages:" << std::endl;
    for (std::map<int, std::string>::const_iterator it = errorPages.begin();
         it != errorPages.end(); ++it) {
        std::cout << "Error Code: " << it->first
                  << " => Page: " << it->second << std::endl;
    }

    std::cout << "\n=== Locations ===" << std::endl;
    for (std::map<std::string, Location>::const_iterator it = locations.begin();
         it != locations.end(); ++it) {
        std::cout << "\nLocation Path: " << it->first << std::endl;
        it->second.printLocation();
    }
}
