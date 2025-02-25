#include "../includes/webserv.hpp"

void post_response(Client& client) {

  if (client.request.size() < client.waitlist[0].content_length) {
    client.status = RECEIVING;
    return ;
  }
  client.status = post_request_header_parser(client);
  if (client.status != OK) {
    std::clog << "Client Status: " << client.status << std::endl;
    return;
  }

  // Extract the body from the request string
  client.waitlist[0].body = client.request.substr(0, client.waitlist[0].content_length);
  client.request.erase(0, client.waitlist[0].content_length);
  client.status = OK;

  if (chdir((client.server->root_directory + "/" + client.server->post_directory).c_str()) == -1) {
    std::cerr << "Page directory not accessible:" << client.server->post_directory << std::endl;
    client.status = ERROR;
    return ;
  }

  if (client.waitlist[0].header_map.at("Content-Type").find("multipart") == 0) {
    post_request_part_handler(client.waitlist[0]);
  } else {
    post_request_simple_handler(client.waitlist[0]);
  }

  client.waitlist[0].response.http_code = 201;
  client.waitlist[0].response.has_content = false;
  http_response(client, client.waitlist[0].response);
  client.status = OK;
}

int post_request_simple_handler(Request& request) {

  std::ofstream outfile("Outfile");
  outfile << request.body;
  outfile.close();
  return (0);
}

int post_request_part_handler(Request& request) {

    std::size_t begin;
    std::size_t end;
    std::size_t filename_pos;
    std::size_t filename_end;
    std::size_t body_end;
    std::string filename;
    std::string subheader;
    std::string boundary = request.header_map.at("boundary=");
    std::string rn = "\r\n";

  while (request.body != boundary + "--\r\n") {

    begin = request.body.find(boundary + rn) + boundary.length() + rn.length(); //end of first boundary
    end = request.body.find("\r\n\r\n", begin) + 2 * rn.length(); //begin of content

    subheader = request.body.substr(begin, end - 1);

    filename_pos = subheader.find("filename=") + 9;
    filename_end = subheader.find('"', filename_pos + 1);
    filename = subheader.substr(filename_pos, filename_end - filename_pos);
    filename.erase(std::remove(filename.begin(), filename.end(), '"'), filename.end());

    body_end = request.body.find(boundary, begin);

    std::ofstream outfile(filename.c_str(), std::ios::out | std::ios::binary);
    if (!outfile.is_open()) {
      std::cerr << "Can't produce outfile" << std::endl;
      return (1);
    }
    outfile << request.body.substr(end, body_end - end - 2);
    outfile.close();
    request.body.erase(request.body.begin(), request.body.begin() + body_end);
  }
  request.body.erase(boundary.length() + 4);
  return (0);
}

int post_request_header_parser(Client& client) {

  if (client.waitlist[0].header_map.count("Content-Type") == 0)
    return (HEADER_INVAL_REGEX_KEY);
  //Checking for multipart and boundary
  if (client.waitlist[0].header_map.at("Content-Type").find("multipart") != std::string::npos) {
      std::size_t boundary_position = client.waitlist[0].header_map.at("Content-Type").find("boundary=");
      if (boundary_position == std::string::npos)
        return (HEADER_INVAL_REGEX_VAL);
      std::string boundary = client.waitlist[0].header_map.at("Content-Type").substr(boundary_position + 9, std::string::npos);
      client.waitlist[0].header_map.insert(std::make_pair("boundary=", "--" + boundary));
  }
  return (OK);
}
