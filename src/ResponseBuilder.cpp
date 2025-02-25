#include "../includes/webserv.hpp"

// loads the http code default paged into buffer
void load_http_code_page(Client& client, Response& response) {

  if (chdir((client.server->root_directory + "/" + client.server->page_directory).c_str()) == -1) {
    std::cerr << "Page directory not accessible:" << client.server->page_directory << std::endl;
    client.status = ERROR;
    return ;
  }

  std::ifstream	infile;
  std::string temp = to_string(response.http_code) + ".html";

  infile.open(temp.c_str());
  response.request_path = ".html";
  if (!infile.is_open()) {
    response.file_content = "<html><body><h1>404 Not Found</h1></body></html>";
    response.http_code = 404;
  }
  response.file_content = std::string((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
}

std::string getMimeType(const std::string &filename) {

  static std::map<std::string, std::string> mimeTypes = {
    {".html", "text/html"},
    {".css", "text/css"},
    {".js", "application/javascript"},
    {".json", "application/json"},
    {".png", "image/png"},
    {".ico", "image/png"},
    {".jpg", "image/jpeg"},
    {".gif", "image/gif"},
    {".svg", "image/svg+xml"},
    {".txt", "text/plain"},
    {".pdf", "application/pdf"},
  };

  size_t dotPos = filename.rfind('.');
  if (dotPos != std::string::npos) {
    std::string extension = filename.substr(dotPos);
    std::map<std::string, std::string>::iterator it = mimeTypes.find(extension);
    if (it != mimeTypes.end()) {
      return (it->second);
    }
  }
  return "application/octet-stream";
}


void http_response(Client& client, Response& response) {

  client.waitlist[0].response.request_path = client.waitlist[0].request_path;

  response.code_string = return_http_code(response.http_code);
  if (response.http_code == 301 || response.http_code == 302 || response.http_code == 307 || response.http_code == 308) {
    redirection_response(response);
    return ;
  }
  if (response.has_content == false) {
    load_http_code_page(client, response);
  }


  response.content_type = getMimeType(client.waitlist[0].response.request_path);
  content_response(response);
}

void redirection_response (Response& response) {
  response.content = http_version;
  response.content.append(" ");
  response.content.append(response.code_string);
  response.content.append(newline);
  response.content.append("Location: ");
  response.content.append(response.redirection_URL);
  response.content.append(newline);
  response.content.append("Connection: close");
  response.content.append(newline);
  response.content.append(newline);
}

void content_response(Response& response) {

  response.content = http_version;
  response.content.append(" ");
  response.content.append(response.code_string);
  response.content.append(newline);
  response.content.append("Content-Type: ");
  response.content.append(response.content_type);
  response.content.append(newline);
  response.content.append("Content-Length: ");
  response.content.append(std::to_string(response.file_content.length()));
  response.content.append(newline);
  response.content.append(newline);
  response.content.append(response.file_content);
  response.content.append(newline);
  response.content.append(newline);
}

std::string return_http_code(int code) {
  switch (code)
  {
  case 200:
    return ("200 OK");
  case 201:
    return ("201 Created");
  case 204:
    return ("204 No Content");
  case 301:
    return ("301 Moved Permanently");
  case 307:
    return ("307 Temporary Redirect");
  case 308:
    return ("308 Permanent Redirect");
  case 400:
    return ("400 Bad Request");
  case 403:
    return ("403 Forbidden");
  case 404:
    return ("404 Not Found");
  case 413:
    return ("413 Content Too Large");
  case 500:
    return ("500 Internal Server Error");
  case 501:
    return ("501 Not Implemented");
  case 505:
    return ("505 HTTP Version Not Supported");

  default:
    return ("404 Not Found");
  }
}
