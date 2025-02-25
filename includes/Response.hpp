#pragma once

#include "webserv.hpp"

struct Response
{
  ssize_t http_code;
  std::string code_string;
  bool has_content;
  bool was_redirected;
  bool was_routed;
  std::string redirection_URL;
  std::string content;
  std::string file_content;
  std::string content_type;
  std::string request_path;

};
