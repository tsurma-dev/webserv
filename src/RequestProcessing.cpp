#include "../includes/webserv.hpp"

bool check_redirection(Client& client, std::string& request_file, Response& response) {
  if (client.server->redirection_table.find(request_file) != client.server->redirection_table.end()) {
    response.redirection_URL = client.server->redirection_table.at(request_file);
    response.was_redirected = true;
    response.http_code = 301;
    return (true);
  }
  return (false);
}

bool is_file_path(Request& request) {
  std::size_t slash = request.request_path.find_last_of("/");
  if (slash == 0 || slash == std::string::npos) {
    return (false);
  }
  // request.request_path.erase(request.request_path.begin());
  return (true);
}


bool set_route(Client& client, std::string &request_file) {
  std::map<std::string, std::string>::const_iterator it =
      client.server->routing_table.find(request_file);
  if (it != client.server->routing_table.end()) {
      request_file = it->second;
      client.waitlist[0].was_routed = true;
      return true;
  }
  for (it = client.server->routing_table.begin(); it != client.server->routing_table.end(); ++it) {
      const std::string &locPath = it->first;  // e.g. "/upload"
      if (locPath.size() > 1 && locPath != "/" &&
          request_file.compare(0, locPath.size(), locPath) == 0)
      {
          std::string remainder = request_file.substr(locPath.size());
          std::string expandedPath = it->second + remainder;
          request_file = expandedPath;
          client.waitlist[0].was_routed = true;
          return true;
      }
  }
  // If no prefix matched, not routed
  client.waitlist[0].was_routed = false;
  return false;
}


bool check_method_server(Client& client) {
  if ((client.server->methods & client.waitlist[0].type) == 0)
    return (false);
  return (true);
}

int set_root_dir(Client& client) {
  return (chdir(client.server->root_directory.c_str()));
}

bool check_method_route(Client& client) {
  Request&  request =  client.waitlist[0];
  Server& server = *client.server;
  TrieNode* match = findBestMatch(server.root, request.request_path);
  if (match == nullptr) {
    return (false);
  }
  if ((match->permissions & request.type) == 0)
    return (false);
  return (true);
}

void process_request(Client& client) {

  if (check_method_server(client) == false) {
    std::clog << "Not implemented method: " << client.waitlist[0].type << std::endl;
    client.waitlist[0].type = INVALID;
  }

  if (set_root_dir(client) == -1) {
    std::clog << "Couldn't set root directory" << std::endl;
    client.waitlist[0].type = INVALID;
  }

  if (check_redirection(client, client.waitlist[0].request_path, client.waitlist[0].response) == true) {
    client.waitlist[0].response.http_code = 301;
    http_response(client, client.waitlist[0].response);
    send_response(client, client.waitlist[0].response);
    client.status = CLOSE;
    return ;
  }

  // Replaces the request path with the one in the routing table
  if (set_route(client, client.waitlist[0].request_path) == false) {
    client.waitlist[0].is_file_path = is_file_path(client.waitlist[0]);
  }


    // if the request wasn't routed and is not a file path, amend the request path to point to the page directory
  if (client.waitlist[0].was_routed == false && client.waitlist[0].is_file_path == false && client.waitlist[0].type == GET) {
      client.waitlist[0].request_path = client.server->page_directory + client.waitlist[0].request_path;
    }

  if (check_method_route(client) == false) {
    std::clog << client.waitlist[0].request_path << "\n";
    std::clog << "Method not allowed on path" << std::endl;
    client.waitlist[0].type = INVALID;
  }

  switch (client.waitlist[0].type)
  {
  case GET:

    if (get_response(client, client.waitlist[0]) == true)
      client.status = CLOSE;
    break;

  case POST:
    if (client.waitlist[0].is_file_path == false) {
      client.waitlist[0].request_path = client.server->post_directory + client.waitlist[0].request_path;
    }
    //TODO: remove stoi
    client.waitlist[0].content_length = std::stoi(client.waitlist[0].header_map.at("Content-Length"));
    if (client.waitlist[0].content_length > client.server->max_body_size) {
      client.status = BODY_TOO_LARGE;
      return ;
    }
    post_response(client);
    if (client.status == RECEIVING)
      return ;
    break;

  case DELETE:
    if (client.waitlist[0].is_file_path == false) {
      client.waitlist[0].request_path = client.server->post_directory + client.waitlist[0].request_path;
    }
    delete_response(client);
    break;

  case HEAD:
    break;

  case INVALID:
    client.waitlist[0].response.http_code = 501;
    client.waitlist[0].response.has_content = false;
    http_response(client, client.waitlist[0].response);
    break;

  default:
    client.waitlist[0].response.http_code = 501;
    client.waitlist[0].response.has_content = false;
    http_response(client, client.waitlist[0].response);
    break;
  }
  send_response(client, client.waitlist[0].response);
}


void send_response(Client& client, Response& response) {
  std::clog << "///////////////////////////////\n" << "client.fd: " << client.fd << "\nResponse:\n" << client.waitlist[0].response.content << std::endl;
  //send the response and delete all temp data
  send(client.fd, response.content.c_str(), response.content.length(), 0);
  client.waitlist.erase(client.waitlist.begin());
}
