#include "../includes/webserv.hpp"



//TODO: replace with 98 function
bool validate_header_key(std::string& key) {
  const std::regex key_regex("^[!#$%&'*+.^_`|~0-9a-zA-Z-]+$");
  return (std::regex_match(key, key_regex));
}

//TODO: replace with 98 function
bool validate_header_value(std::string& value) {
  const std::regex value_regex("^[\\t\\x20-\\x7E]*$");
  return (std::regex_match(value, value_regex));
}

void sanitize_line(std::string& line) {
  // Since readline does not include the newline character, pop the remaining
  // Carriage return, then replace any loose carriage returns that may be
  // included in error.
  if (*(line.end() - 1) == '\r')
    line.pop_back();
  std::replace(line.begin(), line.end(), '\r', ' ');
}

int parse_line(std::string& line, Request& new_request) {

  //Find the mandatory colon that seperates key from content
  size_t colon = line.find(':');
  if (colon == std::string::npos) {
    std::clog << "Header colon not found\n" << line << std:: endl;
    return (HEADER_INVAL_COLON);
  }
  std::string key = line.substr(0, colon);
  std::string value = line.substr(colon + 1);

  size_t start = value.find_first_not_of(" \t");
  size_t end = value.find_last_not_of(" \t");
  value = (start == std::string::npos || end == std::string::npos) ? "" : value.substr(start, end - start + 1);

  if (validate_header_key(key) == false) {
    std::clog << "header key validation failed" << std::endl;
    return (HEADER_INVAL_REGEX_KEY);
  }
  if (validate_header_value(value) == false) {
    std::clog << "header value validation failed" << std::endl;
    return (HEADER_INVAL_REGEX_VAL);
  }

  //Put the "key" and "value" into the hader map. The key does NOT have the colon
  new_request.header_map.emplace(key, value);
  return (OK);
}

void set_type(Request& request) {

  size_t prefix_len = request.start_line.find(' ');
  if (prefix_len == std::string::npos) {
    request.type = INVALID;
    return ;
  }
  std::string temp = request.start_line.substr(0, prefix_len);
  if (temp == "GET")
    request.type = GET;
  else if (temp == "POST")
    request.type = POST;
  else if (temp == "DELETE")
    request.type = DELETE;
  else if (temp == "HEAD")
    request.type = HEAD;
  else {
    request.type = INVALID;
    std::clog << "Unexpected request: " << temp << std::endl;
  }
}

int parse_header(std::string header, Request& new_request) {

  std::istringstream stream(header);
  std::string line;
  int status = 0;

  std::getline(stream, new_request.start_line);
  while (std::getline(stream, line)) {

    if (line.empty())
      break;

    sanitize_line(line);

    status = parse_line(line, new_request);
    if (status != OK)
      return (status);

  }

  set_type(new_request);
  status = set_request_path(new_request);
  return (status);

}

int set_request_path(Request& request) {
  std::size_t start = request.start_line.find("/");
  if (start == std::string::npos)
    return (HEADER_INVAL_REGEX_VAL);
  std::size_t  end  = request.start_line.find(" ", start);
  if (end == std::string::npos)
    return (HEADER_INVAL_REGEX_VAL);
  if (end < start)
    return (HEADER_INVAL_REGEX_VAL);
  request.request_path = request.start_line.substr(start, end - (start));
  return (OK);
}
