#include "../includes/webserv.hpp"


bool get_response(Client& client, Request& request) {

  struct stat stats;
  stat(request.request_path.c_str(), &stats);
  if (S_ISDIR(stats.st_mode) || access(request.request_path.c_str(), R_OK) == -1) {
    request.response.http_code = 404;
    request.response.has_content = false;
    http_response(client, request.response);
  }
  else {
    std::ifstream	infile;
    infile.open(request.request_path.c_str());
    request.response.file_content = std::string((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
    request.response.request_path = request.request_path;
    request.response.has_content = true;
    request.response.http_code = 200;
    http_response(client, request.response);
  }
  return (false);
}
