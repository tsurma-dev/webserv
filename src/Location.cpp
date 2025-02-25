#include "../includes/Location.hpp"
#include "../includes/ParsedServer.hpp"
#include "../includes/webserv.hpp"

// Default Constructor
Location::Location()
    : path(""),
      root(""),
      index(""),
      autoindex(false),
      methods(),
      is_cgi(false),
      cgi_extension(""),
      cgi_pass(""),
      error_pages(),
      client_max_body_size(0),
      return_url() {}

// Constructor that takes a ParsedServer instead of Server
Location::Location(const ParsedServer &server)
    : path(""), // Default path
      root(server.getRoot()),
      index(""),
      autoindex(server.getAutoIndex()),
      methods(server.getAllowedMethods()),
      is_cgi(false),
      cgi_extension(""),
      cgi_pass(""),
      error_pages(server.getErrorPages()),
      client_max_body_size(server.getMaxBodySize()),
      return_url()
{
}

// Copy Constructor
Location::Location(const Location &other) {
    *this = other;
}

// Assignment Operator
Location &Location::operator=(const Location &other) {
    if (this != &other) {
        path                = other.path;
        root                = other.root;
        index               = other.index;
        autoindex           = other.autoindex;
        methods             = other.methods;
        is_cgi              = other.is_cgi;
        cgi_extension       = other.cgi_extension;
        cgi_pass            = other.cgi_pass;
        error_pages         = other.error_pages;
        client_max_body_size= other.client_max_body_size;
        return_url          = other.return_url;
    }
    return *this;
}

// Destructor
Location::~Location() {}

// Getters
const std::string &Location::getPath() const {
    return path;
}

const std::string &Location::getRoot() const {
    return root;
}

const std::string &Location::getIndex() const {
    return index;
}

bool Location::getAutoIndex() const {
    return autoindex;
}

const std::vector<std::string> &Location::getMethods() const {
    return methods;
}

bool Location::isCgi() const {
    return is_cgi;
}

const std::string &Location::getCgiExtension() const {
    return cgi_extension;
}

const std::string &Location::getCgiPass() const {
    return cgi_pass;
}

const std::map<int, std::string> &Location::getErrorPages() const {
    return error_pages;
}

size_t Location::getClientMaxBodySize() const {
    return client_max_body_size;
}

const std::map<int, std::string> &Location::getReturnUrl() const {
    return return_url;
}

// Setters
void Location::setPath(const std::string &path) {
    this->path = path;
}

void Location::setRoot(const std::string &root) {
    this->root = root;
}

void Location::setIndex(const std::string &index) {
    this->index = index;
}

void Location::setAutoIndex(bool autoindex) {
    this->autoindex = autoindex;
}

void Location::setMethods(const std::vector<std::string> &methods) {
    this->methods = methods;
}

void Location::setIsCgi(bool is_cgi) {
    this->is_cgi = is_cgi;
}

void Location::setCgiExtension(const std::string &cgi_extension) {
    this->cgi_extension = cgi_extension;
}

void Location::setCgiPass(const std::string &cgi_pass) {
    this->cgi_pass = cgi_pass;
}

void Location::setErrorPages(const std::map<int, std::string> &error_pages) {
    this->error_pages = error_pages;
}

void Location::setClientMaxBodySize(size_t client_max_body_size) {
    this->client_max_body_size = client_max_body_size;
}

void Location::setReturnUrl(const std::map<int, std::string> &return_url) {
    this->return_url = return_url;
}

// Debug printing
void Location::printLocation() const {
    std::cout << "Path: " << path << std::endl;
    std::cout << "Root: " << root << std::endl;
    std::cout << "Index: " << index << std::endl;
    std::cout << "Autoindex: " << (autoindex ? "on" : "off") << std::endl;

    std::cout << "Allowed Methods: ";
    for (size_t i = 0; i < methods.size(); ++i) {
        std::cout << methods[i] << (i < methods.size() - 1 ? ", " : "");
    }
    std::cout << std::endl;

    std::cout << "Is CGI: " << (is_cgi ? "yes" : "no") << std::endl;
    if (is_cgi) {
        std::cout << "CGI Extension: " << cgi_extension << std::endl;
        std::cout << "CGI Pass: " << cgi_pass << std::endl;
    }

    std::cout << "Error Pages:" << std::endl;
    for (std::map<int, std::string>::const_iterator it = error_pages.begin();
         it != error_pages.end(); ++it) {
        std::cout << "Error Code: " << it->first << " => Page: " << it->second << std::endl;
    }

    std::cout << "Client Max Body Size: " << client_max_body_size << " bytes" << std::endl;

    if (!return_url.empty()) {
        std::cout << "Return URLs:" << std::endl;
        for (std::map<int, std::string>::const_iterator it = return_url.begin();
             it != return_url.end(); ++it) {
            std::cout << "Status Code: " << it->first
                      << " => URL: " << it->second << std::endl;
        }
    }
}
