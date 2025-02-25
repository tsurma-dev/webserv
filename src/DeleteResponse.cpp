#include "../includes/webserv.hpp"

void delete_response(Client& client) {

  std::cout << client.waitlist[0].request_path << std::endl;
  std::ifstream file(client.waitlist[0].request_path);


  if (!file) {
    std::cout << "File not found" << std::endl;
    client.waitlist[0].response.http_code = 204;
    client.waitlist[0].response.has_content = false;
    http_response(client, client.waitlist[0].response);
    return ;
  }

  if (std::remove(client.waitlist[0].request_path.c_str()) == 0) {
    client.waitlist[0].response.http_code = 200;
    client.waitlist[0].response.has_content = false;
    http_response(client, client.waitlist[0].response);
  } else {
      //TODO: add http response
      std::perror("Error deleting file");
  }
  return ;
}
