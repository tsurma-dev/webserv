#include "../includes/Config.hpp"

// Some utility functions your parser uses:
Config::Config(const std::string &configPath)
    : configPath(configPath),
      maxBodySize(10485760),
      globalAutoIndex(false),
      globalDirList("")
{
    // By default, allow GET and HEAD globally
    globalAllowedMethods.push_back("GET");
    globalAllowedMethods.push_back("HEAD");

    parseConfigFile(configPath);
}

Config::Config(const Config &config) {
    *this = config;
}

Config &Config::operator=(const Config &config) {
    if (this == &config)
        return *this;
    servers             = config.servers;
    configPath          = config.configPath;
    errorPages          = config.errorPages;
    maxBodySize         = config.maxBodySize;
    globalAutoIndex     = config.globalAutoIndex;
    globalAllowedMethods= config.globalAllowedMethods;
    globalDirList       = config.globalDirList;
    return *this;
}

Config::~Config() {}

size_t Config::getMaxBodySize() const {
    return maxBodySize;
}

const std::map<int, std::string> &Config::getErrorPages() const {
    return errorPages;
}

bool Config::getGlobalAutoIndex() const {
    return globalAutoIndex;
}

const std::vector<std::string> &Config::getGlobalAllowedMethods() const {
    return globalAllowedMethods;
}

const std::vector<ParsedServer> Config::getServers() const {
    return servers;
}

std::string getFirstToken(const std::string &line) {
    std::istringstream stream(line);
    std::string token;
    stream >> token;
    return token;
}

void Config::printConfig() const {
    std::cout << "=== Global Configuration ===" << std::endl;
    std::cout << "Client Max Body Size: " << maxBodySize << " bytes" << std::endl;

    std::cout << "\nError Pages:" << std::endl;
    for (std::map<int, std::string>::const_iterator it = errorPages.begin();
         it != errorPages.end(); ++it) {
        std::cout << "Error Code: " << it->first << " => Page: " << it->second << std::endl;
    }

    std::cout << "\nGlobal Autoindex: " << (globalAutoIndex ? "on" : "off") << std::endl;

    std::cout << "\nAllowed Methods:" << std::endl;
    for (size_t i = 0; i < globalAllowedMethods.size(); ++i) {
        std::cout << "Method: " << globalAllowedMethods[i] << std::endl;
    }

    std::cout << "\n=== Servers ===" << std::endl;
    for (size_t i = 0; i < servers.size(); ++i) {
        std::cout << "\nServer " << i + 1 << ":" << std::endl;
        servers[i].printServer();
    }
}

void Config::parseConfigFile(const std::string &configPath) {
    std::ifstream file(configPath.c_str());
    if (!file.is_open()) {
        std::cerr << "Error: could not open config file" << std::endl;
        exit(1);
    }
    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty())
            continue;
        line = removeInlineComments(line);
        if (line.empty())
            continue;

        std::string directive = getFirstToken(line);
        if (directive == "server") {
            if (!isValidOpeningBlock(line, "server")) {
                std::cerr << "Error: Invalid syntax in server directive: " << line << std::endl;
                exit(1);
            }
            parseServerBlock(file);
        }
        else if (directive == "dir_list") {
            this->globalDirList = parseDirList(line);
        }
        else if (directive == "error_page") {
            std::map<int, std::string> parsedErrorPages = parseErrorPage(line);
            errorPages.insert(parsedErrorPages.begin(), parsedErrorPages.end());
        }
        else if (directive == "autoindex") {
            globalAutoIndex = parseAutoIndex(line);
        }
        else if (directive == "client_max_body_size") {
            this->maxBodySize = parseBodySize(line);
        }
        else if (directive == "allow") {
            globalAllowedMethods = parseAllowedMethods(line);
        }
        else {
            std::cerr << "Error: Unknown global directive: " << line << std::endl;
            exit(1);
        }
    }
}

void Config::parseServerBlock(std::ifstream &file) {
    std::string line;
    // Create a ParsedServer using the global config as defaults
    ParsedServer server(*this);

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty())
            continue;
        line = removeInlineComments(line);
        if (line.empty())
            continue;

        // If we find a closing brace, push this server and return
        if (isValidClosingBlock(line)) {
            servers.push_back(server);
            return;
        }

        std::string directive = getFirstToken(line);

        if (directive == "server_name") {
            server.setServerName(parseServerName(line));
        }
        else if (directive == "dir_list") {
            server.setDirList(parseDirList(line));
        }
        else if (directive == "host") {
            server.setHost(parseHost(line));
        }
        else if (directive == "root") {
            server.setRoot(parseRoot(line));
        }
        else if (directive == "listen") {
            server.setPort(parsePort(line));
        }
        else if (directive == "error_page") {
            std::map<int, std::string> parsedErrorPages = parseErrorPage(line);
            server.setErrorPages(parsedErrorPages);
        }
        else if (directive == "index") {
            server.setIndex(parseIndex(line));
        }
        else if (directive == "autoindex") {
            server.setAutoIndex(parseAutoIndex(line));
        }
        else if (directive == "client_max_body_size") {
            server.setMaxBodySize(parseBodySize(line));
        }
        else if (directive == "allow") {
            server.setAllowedMethods(parseAllowedMethods(line));
        }
        else if (directive == "location") {
            if (!isValidOpeningBlockLocation(line, "location")) {
                std::cerr << "Error: Invalid syntax in location directive: " << line << std::endl;
                exit(1);
            }
            parseLocationBlock(file, server, line);
        }
        else {
            std::cerr << "Error: Unknown directive in server block: " << line << std::endl;
            exit(1);
        }
    }
    std::cerr << "Error: Missing closing '}' for server block" << std::endl;
    exit(1);
}

void Config::parseLocationBlock(std::ifstream &file, ParsedServer &server, const std::string &openLine) {
    // Essentially the same as before, but note the second parameter is now `ParsedServer &server`
    std::string line = trim(openLine);
    line = removeInlineComments(line);
    if (!isValidOpeningBlockLocation(line, "location")) {
        std::cerr << "Error: Invalid syntax in location directive: " << line << std::endl;
        exit(1);
    }
    std::istringstream stream(line);
    std::string token, path, extra;
    stream >> token >> path >> extra;
    if (token != "location") {
        std::cerr << "Error: Expected 'location' directive, found: " << token << std::endl;
        exit(1);
    }
    if (path.empty() || path[0] != '/') {
        std::cerr << "Error: Location path must start with '/': " << path << std::endl;
        exit(1);
    }
    if (extra != "{") {
        std::cerr << "Error: Missing '{' after location path: " << line << std::endl;
        exit(1);
    }
    std::string remaining;
    stream >> remaining;
    if (!remaining.empty()) {
        std::cerr << "Error: Unexpected tokens after '{' in location directive: " << line << std::endl;
        exit(1);
    }

    Location location(server);
    location.setPath(path);

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;
        line = removeInlineComments(line);
        if (line.empty()) continue;

        if (isValidClosingBlock(line)) {
            server.addLocation(path, location);
            return;
        }

        std::string directive = getFirstToken(line);

        if (directive == "root")
            location.setRoot(parseRoot(line));
        else if (directive == "index")
            location.setIndex(parseIndex(line));
        else if (directive == "autoindex")
            location.setAutoIndex(parseAutoIndex(line));
        else if (directive == "client_max_body_size")
            location.setClientMaxBodySize(parseBodySize(line));
        else if (directive == "allow")
            location.setMethods(parseAllowedMethods(line));
        else if (directive == "cgi")
            location.setIsCgi(parseCgi(line));
        else if (directive == "cgi_extension") {
            location.setIsCgi(true);
            location.setCgiExtension(parseCgiExtension(line));
        }
        else if (directive == "cgi_pass")
            location.setCgiPass(parseCgiPass(line));
        else if (directive == "return")
            location.setReturnUrl(parseReturnUrl(line));
        else {
            std::cerr << "Error: Unknown directive in location block: " << line << std::endl;
            exit(1);
        }
    }
    std::cerr << "Error: Missing closing '}' for location block" << std::endl;
    exit(1);
}
