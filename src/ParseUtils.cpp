#include "../includes/ParseUtils.hpp"

bool isFileExecutable(const std::string &path)
{
	struct stat fileInfo;
	if (stat(path.c_str(), &fileInfo) != 0)
	{
		return false;
	}

	if (!(fileInfo.st_mode & S_IXUSR))
	{
		return false;
	}

	return true;
}

bool fileExistandHTML(const std::string &path)
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
		return false;
	size_t extPos = path.find_last_of(".");
	if (extPos == std::string::npos || path.substr(extPos) != ".html")
		return false;
	return true;
}

bool isAllDigits(const std::string &str)
{
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}

bool isValidOpeningBlock(const std::string &line, const std::string &directive)
{
	std::istringstream stream(line);
	std::string token, brace;

	stream >> token;
	if (token != directive)
		return false;

	stream >> brace;
	if (brace != "{")
		return false;

	std::string extra;
	if (stream >> extra)
		return false;

	return true;
}

bool isValidOpeningBlockLocation(const std::string &line, const std::string &directive)
{
	std::istringstream stream(line);
	std::string token,path, brace;

	stream >> token;
	if (token != directive)
		return false;
	stream >> path;
	stream >> brace;
	if (brace != "{")
		return false;

	std::string extra;
	if (stream >> extra)
		return false;

	return true;
}

bool isValidClosingBlock(const std::string &line)
{
	return line == "}";
}

std::string removeInlineComments(const std::string &str)
{
	size_t pos = str.find("#");
	return (pos == std::string::npos) ? str : str.substr(0, pos);
}

std::string trim(const std::string &str)
{
	size_t first = str.find_first_not_of(" \t\n\r\f\v");
	size_t last = str.find_last_not_of(" \t\n\r\f\v");
	return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
}

bool isIPv4(const std::string &ip)
{
	int num, dots = 0;
	std::string::const_iterator it = ip.begin();
	std::string segment;

	while (it != ip.end())
	{
		if (*it == '.')
		{
			if (segment.empty() || (num = atoi(segment.c_str())) < 0 || num > 255)
			{
				return false;
			}
			segment.clear();
			dots++;
		}
		else if (std::isdigit(*it))
		{
			segment += *it;
		}
		else
		{
			return false;
		}
		++it;
	}

	if (segment.empty() || (num = atoi(segment.c_str())) < 0 || num > 255)
	{
		return false;
	}

	return dots == 3;
}

bool isValidHostname(const std::string &hostname)
{
	if (hostname.empty() || hostname.size() > 253)
	{
		return false;
	}

	std::string::const_iterator it = hostname.begin();
	std::string segment;
	bool hyphenAllowed = false;

	while (it != hostname.end())
	{
		if (*it == '.')
		{
			if (segment.empty() || !hyphenAllowed)
			{
				return false;
			}
			segment.clear();
			hyphenAllowed = false;
		}
		else if (std::isalnum(*it))
		{
			segment += *it;
			hyphenAllowed = true;
		}
		else if (*it == '-' && !segment.empty())
		{
			segment += *it;
		}
		else
		{
			return false;
		}
		++it;
	}

	return !segment.empty() && hyphenAllowed;
}

bool isValidHost(const std::string &host)
{
	return isIPv4(host) || isValidHostname(host);
}

// Parsing functions

size_t parseBodySize(const std::string &str)
{
	// Check if the line ends with semicolon
	if (str[str.size() - 1] != ';')
	{
		std::cerr << "Error: Missing semicolon at the end of the directive" << std::endl;
		exit(1);
	}
	std::string trimmedStr = str.substr(0, str.size() - 1);
	// Checking for the negative value
	if (trimmedStr.find_first_of("-") == 0)
	{
		std::cerr << "Error: Body size cannot be negative" << std::endl;
		exit(1);
	}
	const size_t MAX_SIZE = 50 * 1024 * 1024;
	size_t start = trimmedStr.find_first_of("0123456789");
	if (start == std::string::npos)
	{
		std::cerr << "Error: Invalid body size format" << std::endl;
		exit(1);
	}
	size_t end = trimmedStr.find_first_not_of("0123456789", start);
	std::string numberPart = trimmedStr.substr(start, end - start);
	if (numberPart.empty() || !isAllDigits(numberPart))
	{
		std::cerr << "Error: Invalid body size format" << std::endl;
		exit(1);
	}
	size_t size = 0;
	try
	{
		std::stringstream ss(numberPart);
		ss >> size;

		if (ss.fail() || !ss.eof()) {
			std::cerr << "Error: Invalid number format: " << numberPart << std::endl;
			exit(1);
	}

	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: Invalid number in body size" << std::endl;
		exit(1);
	}
	// Reject zero as a body size
	if (size == 0)
	{
		std::cerr << "Error: Body size cannot be zero" << std::endl;
		exit(1);
	}
	// Check for unit
	std::string unit = trimmedStr.substr(end);
	unit = trim(unit); // Remove surrounding whitespace
	if (unit == "K" || unit == "k")
	{
		size *= 1024;
	}
	else if (unit == "M" || unit == "m")
	{
		size *= 1024 * 1024;
	}
	else if (unit == "G" || unit == "g")
	{
		size *= 1024 * 1024 * 1024;
	}
	else if (!unit.empty())
	{
		std::cerr << "Error: Invalid unit in body size: " << unit << std::endl;
		exit(1);
	}
	// Cap the size to 50M (50 * 1024 * 1024 bytes)
	if (size > MAX_SIZE)
	{
		std::cerr << "Warning: Body size exceeds 50M, setting to 50M" << std::endl;
		size = MAX_SIZE;
	}
	return size;
}

std::map<int, std::string> parseErrorPage(const std::string &str)
{
	std::map<int, std::string> errorPage;
	if (str[str.size() - 1] != ';')
	{
		std::cerr << "Error: Missing semicolon at the end of error_page directive" << std::endl;
		exit(1);
	}
	std::string trimmedStr = str.substr(0, str.size() - 1);
	std::istringstream stream(trimmedStr);
	std::string token;
	stream >> token;
	if (token != "error_page")
	{
		std::cerr << "Error: Invalid directive, expected 'error_page'" << std::endl;
		exit(1);
	}
	int errorCode = 0;
	if (!(stream >> errorCode) || errorCode < 100 || errorCode > 599)
	{
		std::cerr << "Error: Invalid or missing HTTP error code in error_page directive" << std::endl;
		exit(1);
	}
	std::string filePath;
	if (!(stream >> filePath) || filePath.empty())
	{
		std::cerr << "Error: Missing file path in error_page directive" << std::endl;
		exit(1);
	}
	if (!fileExistandHTML(filePath))
	{
		std::cerr << "Error: File does not exist or is not a valid HTML file: " << filePath << std::endl;
		exit(1);
	}
	if (stream >> token)
	{
		std::cerr << "Error: Unexpected token '" << token << "' in error_page directive" << std::endl;
		exit(1);
	}
	errorPage[errorCode] = filePath;
	return errorPage;
}

std::vector<std::string> parseAllowedMethods(const std::string &str)
{
	std::set<std::string> validMethods;
	validMethods.insert("GET");
	validMethods.insert("POST");
	validMethods.insert("DELETE");
	std::vector<std::string> methods;

	if (str[str.size() - 1] != ';')
	{
		std::cerr << "Error: Missing semicolon at the end of error_page directive" << std::endl;
		exit(1);
	}
	std::string trimmedStr = str.substr(0, str.size() - 1);
	std::istringstream stream(trimmedStr);
	std::string token;
	stream >> token;
	if (token != "allow")
	{
		std::cerr << "Error: Invalid directive, expected 'allow'" << std::endl;
		exit(1);
	}
	while (stream >> token)
	{
		if (validMethods.find(token) == validMethods.end())
		{
			std::cerr << "Error: Invalid HTTP method '" << token << "' in allow directive" << std::endl;
			exit(1);
		}
		methods.push_back(token);
	}
	if (methods.empty())
	{
		std::cerr << "Error: No HTTP methods specified in allow directive" << std::endl;
		exit(1);
	}
	return methods;
}

bool parseAutoIndex(const std::string &str)
{
	if (str[str.size() - 1] != ';')
	{
		std::cerr << "Error: Missing semicolon at the end of autoindex directive" << std::endl;
		exit(1);
	}
	std::string trimmedStr = str.substr(0, str.size() - 1);
	std::istringstream stream(trimmedStr);
	std::string token;
	stream >> token;
	if (token != "autoindex")
	{
		std::cerr << "Error: Invalid directive, expected 'autoindex'" << std::endl;
		exit(1);
	}
	stream >> token;
	if (token != "on" && token != "off")
	{
		std::cerr << "Error: Invalid value '" << token << "' in autoindex directive. Expected 'on' or 'off'" << std::endl;
		exit(1);
	}
	std::string remainingToken;
	if (stream >> remainingToken)
	{
		std::cerr << "Error: Unexpected token '" << remainingToken << "' in autoindex directive" << std::endl;
		exit(1);
	}
	return (token == "on");
}

std::string parseServerName(std::string &str)
{
	if (str[str.size() - 1] != ';')
	{
		std::cerr << "Error: Missing semicolon at the end of autoindex directive" << std::endl;
		exit(1);
	}
	str = trim(str);
	std::string trimmedStr = str.substr(0, str.size() - 1);
	std::istringstream stream(trimmedStr);
	std::string directive, serverName;
	stream >> directive;
	if (directive != "server_name")
	{
		std::cerr << "Error: Invalid directive, expected 'server_name'" << std::endl;
		exit(1);
		std::vector<std::string> host;
	}
	if (!(stream >> serverName) || serverName.empty())
	{
		std::cerr << "Error: Missing server name in server_name directive" << std::endl;
		exit(1);
	}
	if (serverName.length() > 253)
	{
		std::cerr << "Error: Server name exceeds the maximum allowed length of 253 characters: "
				  << serverName << std::endl;
		exit(1);
	}
	std::string remainingToken;
	if (stream >> remainingToken)
	{
		std::cerr << "Error: Unexpected token '" << remainingToken << "' in server_name directive" << std::endl;
		exit(1);
	}
	return serverName;
}

// std::string parseHost(std::string &str) {
// 	if (str[str.size() - 1] != ';')
// 	{
// 		std::cerr << "Error: Missing semicolon at the end of server_name directive" << std::endl;
// 		exit(1);
// 	}
// 	std::string trimmedStr = str.substr(0, str.size() - 1);
// 	trimmedStr = trim(trimmedStr);
// }

std::vector<std::string> parseHost(const std::string &str)
{
	if (str[str.size() - 1] != ';')
	{
		std::cerr << "Error: Missing semicolon at the end of host directive" << std::endl;
		exit(1);
	}
	std::string trimmedStr = str.substr(0, str.size() - 1);
	std::istringstream stream(trimmedStr);
	std::string token;
	std::vector<std::string> hosts;
	stream >> token;
	if (token != "host")
	{
		std::cerr << "Error: Invalid directive, expected 'host'" << std::endl;
		exit(1);
	}
	while (stream >> token)
	{
		if (!isValidHost(token))
		{
			std::cerr << "Error: Invalid host value: " << token << std::endl;
			exit(1);
		}
		hosts.push_back(token);
	}
	if (hosts.empty())
	{
		std::cerr << "Error: No host vlues specified in host directive" << std::endl;
		exit(1);
	}
	return hosts;
}

std::vector<int> parsePort(const std::string &str)
{
	std::vector<int> ports;
	if (str[str.size() - 1] != ';')
	{
		std::cerr << "Error: Missing semicolon at the end of port directive" << std::endl;
		exit(1);
	}
	std::string trimmedStr = str.substr(0, str.size() - 1);
	std::istringstream stream(trimmedStr);
	std::string token;
	stream >> token;
	if (token != "listen")
	{
		std::cerr << "Error: Invalid directive, expected 'port'" << std::endl;
		exit(1);
	}
	while (stream >> token)
	{
		if (!isAllDigits(token))
		{
			std::cerr << "Error: Invalid port number: " << token << std::endl;
			exit(1);
		}
		int port = atoi(token.c_str());
		if (port < 1 || port > 65535)
		{
			std::cerr << "Error: Port number out of range (1-65535)" << std::endl;
			exit(1);
		}
		if (std::find(ports.begin(), ports.end(), port) == ports.end())
		{
			ports.push_back(port);
		}
	}
	if (ports.empty())
	{
		std::cerr << "Error: No host values specified in port directive" << std::endl;
		exit(1);
	}
	return ports;
}

std::string parseDirList(const std::string &str)
{
	if (str[str.size() - 1] != ';')
	{
		std::cerr << "Error: Missing semicolon at the end of dir_list directive" << std::endl;
		exit(1);
	}
	std::string trimmedStr = str.substr(0, str.size() - 1);
	std::istringstream stream(trimmedStr);
	std::string token;
	stream >> token;
	if (token != "dir_list")
	{
		std::cerr << "Error: Invalid directive, expected 'dir_list'" << std::endl;
		exit(1);
	}
	std::string filePath;
	if (!(stream >> filePath) || filePath.empty())
	{
		std::cerr << "Error: Missing file path in dir_list directive" << std::endl;
		exit(1);
	}
	if (stream >> token)
	{
		std::cerr << "Error: Unexpected token '" << token << "' in dir_list directive" << std::endl;
		exit(1);
	}
	if (!fileExistandHTML(filePath))
	{
		std::cerr << "Error: File does not exist or is not a valid HTML file: " << filePath << std::endl;
		exit(1);
	}

	return filePath;
}

std::string parseRoot(const std::string &str)
{
	if (str[str.size() - 1] != ';')
	{
		std::cerr << "Error: Missing semicolon at the end of dir_list directive " << std::endl;
		exit(1);
	}
	std::string trimmedStr = str.substr(0, str.size() - 1);
	std::istringstream stream(trimmedStr);
	std::string token;

	stream >> token;
	if (token != "root")
	{
		std::cerr << "Error: Invalid directive, expected 'root'" << std::endl;
		exit(1);
	}
	std::string root;
	if (!(stream >> root) || root.empty())
	{
		std::cerr << "Error: Missing file path in root directive" << std::endl;
		exit(1);
	}
	if (stream >> token)
	{
		std::cerr << "Error: Unexpected token '" << token << "' in root directive" << std::endl;
		exit(1);
	}
	return root;
}

std::string parseIndex(const std::string &str)
{
	if (str[str.size() - 1] != ';')
	{
		std::cerr << "Error: Missing semicolon at the end of Index directive " << std::endl;
		exit(1);
	}
	std::string trimmedStr = str.substr(0, str.size() - 1);
	std::istringstream stream(trimmedStr);
	std::string token;

	stream >> token;
	if (token != "index")
	{
		std::cerr << "Error: Invalid directive, expected 'index'" << std::endl;
		exit(1);
	}
	std::string index;
	if (!(stream >> index) || index.empty())
	{
		std::cerr << "Error: Missing file path in index directive" << std::endl;
		exit(1);
	}

	if (index[0] == '/')
	{
		std::cerr << "Error: Absolute paths are not allowed in index directive: " << index << std::endl;
		exit(1);
	}

	if (stream >> token)
	{
		std::cerr << "Error: Unexpected token '" << token << "' in root directive" << std::endl;
		exit(1);
	}
	return index;
}

bool parseCgi(const std::string &str)
{
	if (str[str.size() - 1] != ';')
	{
		std::cerr << "Error: Missing semicolon at the end of autoindex directive" << std::endl;
		exit(1);
	}
	std::string trimmedStr = str.substr(0, str.size() - 1);
	std::istringstream stream(trimmedStr);
	std::string token;
	stream >> token;
	if (token != "cgi")
	{
		std::cerr << "Error: Invalid directive, expected 'cgi'" << std::endl;
		exit(1);
	}
	stream >> token;
	if (token != "on" && token != "off")
	{
		std::cerr << "Error: Invalid value '" << token << "' in cgi directive. Expected 'on' or 'off'" << std::endl;
		exit(1);
	}
	std::string remainingToken;
	if (stream >> remainingToken)
	{
		std::cerr << "Error: Unexpected token '" << remainingToken << "' in cgi directive" << std::endl;
		exit(1);
	}
	return (token == "on");
}

std::map<int, std::string> parseReturnUrl(const std::string &str)
{
	std::map<int, std::string> returnMap;
	if (str[str.size() - 1] != ';')
	{
		std::cerr << "Error: Missing semicolon at the end of return directive" << std::endl;
		exit(1);
	}
	std::string trimmedStr = str.substr(0, str.size() - 1);
	std::istringstream stream(trimmedStr);
	std::string token;
	stream >> token;
	if (token != "return")
	{
		std::cerr << "Error: Invalid directive, expected 'return'" << std::endl;
		exit(1);
	}
	int statusCode;
	if (!(stream >> statusCode) || statusCode < 300 || statusCode > 399)
	{
		std::cerr << "Error: Invalid or missing HTTP status code in return directive. Only 3xx codes are allowed." << std::endl;
		exit(1);
	}
	std::string url;
	if (!(stream >> url) || url.empty())
	{
		std::cerr << "Error: Missing URL in return directive" << std::endl;
		exit(1);
	}
	if (stream >> token)
	{
		std::cerr << "Error: Unexpected token '" << token << "' in return directive" << std::endl;
		exit(1);
	}
	returnMap[statusCode] = url;

	return returnMap;
}

std::string parseCgiPass(const std::string &str)
{
	if (str[str.size() - 1] != ';')
	{
		std::cerr << "Error: Missing semicolon at the end of cgi_pass directive" << std::endl;
		exit(1);
	}

	std::string trimmedStr = str.substr(0, str.size() - 1);
	std::istringstream stream(trimmedStr);
	std::string token;

	stream >> token;
	if (token != "cgi_pass")
	{
		std::cerr << "Error: Invalid directive, expected 'cgi_pass'" << std::endl;
		exit(1);
	}

	std::string cgiPath;
	if (!(stream >> cgiPath) || cgiPath.empty())
	{
		std::cerr << "Error: Missing path in cgi_pass directive" << std::endl;
		exit(1);
	}

	if (stream >> token)
	{
		std::cerr << "Error: Unexpected token '" << token << "' in cgi_pass directive" << std::endl;
		exit(1);
	}

	if (cgiPath[0] != '/')
	{
		std::cerr << "Error: cgi_pass path must be an absolute path: " << cgiPath << std::endl;
		exit(1);
	}

	if (!isFileExecutable(cgiPath)) {
		std::cerr << "Error: File does not exist or is not executable: " << cgiPath << std::endl;
		exit(1);
	}

	return cgiPath;
}

std::string parseCgiExtension(const std::string& str)
{
	if (str[str.size() - 1] != ';')
	{
		std::cerr << "Error: Missing semicolon at the end of cgi_extension directive" << std::endl;
		exit(1);
	}
	std::string trimmedStr = str.substr(0, str.size() - 1);
	std::istringstream stream(trimmedStr);
	std::string token;

	stream >> token;
	if (token != "cgi_extension")
	{
		std::cerr << "Error: Invalid directive, expected 'cgi_extension'" << std::endl;
		exit(1);
	}

	std::string extension;
	if (!(stream >> extension) || extension.empty())
	{
		std::cerr << "Error: Missing or invalid CGI extension" << std::endl;
		exit(1);
	}

	// Ensure no extra tokens
	if (stream >> token)
	{
		std::cerr << "Error: Unexpected token '" << token << "' in cgi_extension directive" << std::endl;
		exit(1);
	}

	// Validate the extension format (e.g., must start with a dot and contain valid characters)
	if (extension[0] != '.' || extension.find_first_not_of(".abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") != std::string::npos)
	{
		std::cerr << "Error: Invalid CGI extension format: " << extension << std::endl;
		exit(1);
	}

	return extension;
}
