#include "../includes/webserv.hpp"

int new_client(std::vector<pollfd>& fd_vec, std::map<int, Server>& server_map, std::map<int, Client>& client_map, std::size_t& i) {
    std::map<int, Server>::iterator it = server_map.find(fd_vec[i].fd); //find server in the map through the fd
  int client_fd = accept((*it).second.server_socket, 0,0); // accept the client on the server
  if (client_fd < 0)
    return (1);
  client_add_vec(client_fd, fd_vec);
  client_add_map(client_map, client_fd, &((*it).second));
  return (OK);
}

//Removes the Client from the Map and vector does NOT close any connections
void client_remove(std::size_t& i, std::map<int, Client>& client_map, std::vector<pollfd>& fd_vec) {
  std::clog << "Removing Client: " << i << " on fd: " << fd_vec[i].fd << std::endl;
  client_map.erase(fd_vec[i].fd);
  fd_vec.erase(fd_vec.begin() + i);
  --i;
  std::clog << "Client removed" << std::endl;
  return ;
}

void client_add_vec(int client_fd, std::vector<pollfd>& fd_vec) {
  pollfd client_poll_fd;
  client_poll_fd.fd = client_fd;
  client_poll_fd.events = POLLIN;
  client_poll_fd.revents = 0;
  fd_vec.push_back(client_poll_fd);
  std::clog << "New Client added: " << fd_vec.size() << "\nFD: " << client_fd << std::endl;
  return ;
}


void client_add_map(std::map<int, Client>& client_map, int fd, Server* server) {
  Client new_client;

  new_client.fd = fd;
  new_client.status = OK;
  new_client.server = server;
  client_map.insert(std::make_pair(fd, new_client));
  return ;
}

void client_error_message(size_t i, int fd, int status) {

  switch (status)
  {
  case DISCONNECTED:
    std::clog << "Client: " << i << "\nFD: " << fd << "\nAction: Disconnected" << std::endl;
    break;

  case ERRPOLL:
    std::clog << "Client " << i << "\nFD: " << fd << "\nAction: POLLERR" << std::endl;
    break;

  case ERROR:
    std::clog << "Client " << i << "\nFD: " << fd << "\nAction: Error" << std::endl;
    break;

  case HUNGUP:
    std::clog << "Client " << i << "\nFD: " << fd << "\nAction: Hung up" << std::endl;
    break;

  case CLOSE:
    std::clog << "Client " << i << "\nFD: " << fd << "\nAction: Server closed connection" << std::endl;
    break;

  case POLLINVALID:
    std::clog << "Client " << i << "\nFD: " << fd << "\nAction: Invalid" << std::endl;
    break;

  //TODO: send error 400
  case HEADER_INVAL_COLON:
    std::clog << "Client " << i << "\nFD: " << fd << "\nAction: Malformed Header" << std::endl;
    break;

  case HEADER_INVAL_REGEX_KEY:
    std::clog << "Client " << i << "\nFD: " << fd << "\nAction: Malformed Header" << std::endl;
    break;

  case HEADER_INVAL_REGEX_VAL:
    std::clog << "Client " << i << "\nFD: " << fd << "\nAction: Malformed Header" << std::endl;
    break;

  case HEADER_INVAL_SIZE:
    std::clog << "Client " << i << "\nFD: " << fd << "\nAction: Malformed Header" << std::endl;
    break;

  //TODO: send error 413
  case BODY_TOO_LARGE:
    std::clog << "Client " << i << "\nFD: " << fd << "\nAction: Content too large" << std::endl;
    break;

  default:
    std::cerr << "Unknown Error status on " << i << " with fd " << fd << "\nError: " << status << std::endl;
    break;
  }
}
