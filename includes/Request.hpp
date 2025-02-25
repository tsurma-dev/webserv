#pragma once

#include "webserv.hpp"
#include "Response.hpp"

struct Request {

  unsigned char type;
  bool was_routed;
  bool is_file_path;
  std::size_t header_length;
  std::size_t content_length;
  std::string start_line;
  std::string body;
  std::string request_path;
  Response response;
  std::map<std::string, std::string> header_map;
};
