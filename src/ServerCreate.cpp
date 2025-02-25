#include "../includes/webserv.hpp"

int getSocket(std::vector<pollfd>& fd_vec, int port) {
    struct sockaddr_in addr;
    int listening_socket;

    listening_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listening_socket == -1)
        return (-1);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    int option = 1;
    setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    if (bind(listening_socket, (sockaddr *)&addr, sizeof(addr)) == -1)
        return (-1);

    if (listen(listening_socket, MAX_CLIENTS) < 0)
        return (-1);

    pollfd server_fd;
    server_fd.fd = listening_socket;
    server_fd.events = POLLIN;
    server_fd.revents = 0;
    fd_vec.push_back(server_fd);
    return (listening_socket);
}

unsigned char convertAllowedMethods(const std::vector<std::string>& methods) {
    unsigned char bitmask = 0;
    for (size_t i = 0; i < methods.size(); ++i) {
        if (methods[i] == "GET")
            bitmask |= GET;
        else if (methods[i] == "POST")
            bitmask |= POST;
        else if (methods[i] == "DELETE")
            bitmask |= DELETE;
    }
    return bitmask;
}

int createServersFromConfig(std::vector<pollfd>& fd_vec,
                            std::map<int, Server>& server_map,
                            const Config& config)
{
    const std::vector<ParsedServer>& parsedServers = config.getServers();

    for (size_t i = 0; i < parsedServers.size(); ++i) {
        Server new_server;

        new_server.server_name     = parsedServers[i].getServerName();
        new_server.root_directory  = parsedServers[i].getRoot();
        new_server.page_directory  = parsedServers[i].getIndex();
        new_server.max_body_size   = parsedServers[i].getMaxBodySize();
        new_server.methods         = convertAllowedMethods(parsedServers[i].getAllowedMethods());

        // /upload location sets the post_directory if found
        const std::map<std::string, Location>& locs = parsedServers[i].getLocations();
        std::map<std::string, Location>::const_iterator up = locs.find("/upload");
        if (up != locs.end())
            new_server.post_directory = up->second.getRoot();
        else
            new_server.post_directory = new_server.root_directory;

        new_server.root = new TrieNode();
        const std::map<std::string, Location>& locations = parsedServers[i].getLocations();

        for (std::map<std::string, Location>::const_iterator it = locations.begin();
             it != locations.end(); ++it)
        {
            // Build a file-system path to insert into the trie
            std::string fileSystemPath = it->second.getRoot().empty()
                                         ? new_server.root_directory
                                         : it->second.getRoot();

            unsigned char locBitmask = convertAllowedMethods(it->second.getMethods());
            insert(new_server.root, fileSystemPath, locBitmask);

            // If there's a return directive, store the redirection
            if (!it->second.getReturnUrl().empty())
                new_server.redirection_table.insert(
                    std::make_pair(it->first, it->second.getReturnUrl().begin()->second));

            // Always add to the routing table:
            // If there's an index, append it; otherwise just store the root path.
            std::string fullPath = fileSystemPath;
            if (!it->second.getIndex().empty()) {
                fullPath += "/";
                fullPath += it->second.getIndex();
            }
            new_server.routing_table.insert(std::make_pair(it->first, fullPath));
        }

        new_server.errorPages = parsedServers[i].getErrorPages();

        // Create sockets for each port
        const std::vector<int>& ports = parsedServers[i].getPorts();
        for (size_t j = 0; j < ports.size(); ++j) {
            int sock = getSocket(fd_vec, ports[j]);
            if (sock == -1) {
                std::cerr << "Error creating socket on port " << ports[j] << std::endl;
                continue;
            }
            new_server.server_socket = sock;
            new_server.ports.push_back(ports[j]);
        }
        server_map.insert(std::make_pair(new_server.server_socket, new_server));
    }
    return OK;
}

void printServer(const Server& srv)
{
    std::cout << "=== Server Info ===" << std::endl;
    std::cout << "server_socket: "      << srv.server_socket << std::endl;
    std::cout << "server_name: "        << srv.server_name   << std::endl;
    std::cout << "ports: ";
    for (size_t i = 0; i < srv.ports.size(); ++i) {
        std::cout << srv.ports[i];
        if (i < srv.ports.size() - 1)
            std::cout << ", ";
    }
    std::cout << std::endl;
    std::cout << "methods (bitmask): "  << static_cast<int>(srv.methods) << std::endl;
    std::cout << "max_body_size: "      << srv.max_body_size << std::endl;
    std::cout << "root (TrieNode*): "   << srv.root << std::endl;
    std::cout << "root_directory: "     << srv.root_directory << std::endl;
    std::cout << "page_directory: "     << srv.page_directory << std::endl;
    std::cout << "post_directory: "     << srv.post_directory << std::endl;

    std::cout << "\nrouting_table:" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = srv.routing_table.begin();
         it != srv.routing_table.end(); ++it)
    {
        std::cout << "  " << it->first << " => " << it->second << std::endl;
    }

    std::cout << "\nredirection_table:" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = srv.redirection_table.begin();
         it != srv.redirection_table.end(); ++it)
    {
        std::cout << "  " << it->first << " => " << it->second << std::endl;
    }

    std::cout << "\nerrorPages:" << std::endl;
    for (std::map<int, std::string>::const_iterator it = srv.errorPages.begin();
         it != srv.errorPages.end(); ++it)
    {
        std::cout << "  " << it->first << " => " << it->second << std::endl;
    }
    std::cout << "====================\n" << std::endl;
}
