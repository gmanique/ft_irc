#include "Server.hpp"

Server::Server() : _port(6667), _password(""), _serverFd(-1) {
	DEBUG(LOG_TRACE << "[SERVER] Creating struct with port `" << _port << "` and password `" << _password << "`.";);
}

Server::Server(int port, const std::string &password) : _port(port), _password(password), _serverFd(-1) {
	DEBUG(LOG_TRACE << "[SERVER] Creating struct with port `" << _port << "` and password `" << _password << "`.";);
}

Server::~Server() {
	if (_serverFd != -1)
		close(_serverFd);
	DEBUG(LOG_TRACE << "[SERVER] Destroying struct.";);
}
		
void	Server::setPort(int port) {
	_port = port;
}
		
void	Server::setPassword(const std::string &password) {
	_password = password;
}

int	Server::getPort() const {
	return (_port);
}

const std::string &Server::getPassword() const {
	return (_password);
}


int	Server::init() {
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd < 0) {
		LOG_ERR << "Socket creation failed.";
		return (MEMORY_ERROR);
	}

	int opt = 1;
	if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		LOG_ERR << "Setsockopt failed.";
		return (MEMORY_ERROR);
	}

	if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) < 0) {
		LOG_ERR << "Fcntl failed.";
		return (MEMORY_ERROR);
	}

	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(_port);

	if (bind(_serverFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		LOG_ERR << "Bind failed, is port already used ?";
		return (MEMORY_ERROR);
	}

	if (listen(_serverFd, SOMAXCONN) < 0) {
		LOG_ERR << "Listen failed.";
		return (MEMORY_ERROR);
	}

	pollfd serverPollFd;
	serverPollFd.fd = _serverFd;
	serverPollFd.events = POLLIN;
	serverPollFd.revents = 0;
	_pollFds.push_back(serverPollFd);

	LOG_INFO << "Serveur initialisé sur le port " << _port;
	return (SUCCESS);
}

int	Server::run() {

	LOG_INFO << "Server starting";
	
	return (SUCCESS);
}
