#include "Server.hpp"

Server::Server() : _port(6667), _password("") {
	DEBUG(std::cout << "[SERVER] Creating struct with port `" << _port << "` and password `" << _password << "`.\n";);
}

Server::Server(uint16_t	port) : _port(port), _password("") {
	DEBUG(std::cout << "[SERVER] Creating struct with port `" << _port << "` and password `" << _password << "`.\n";);
}


Server::Server(uint16_t	port, std::string password) : _port(port), _password(password) {
	DEBUG(std::cout << "[SERVER] Creating struct with port `" << _port << "` and password `" << _password << "`.\n";);
}

Server::Server(std::string password) : _port(6667), _password(password) {
	DEBUG(std::cout << "[SERVER] Creating struct with port `" << _port << "` and password `" << _password << "`.\n";);
}

Server::~Server() {
	DEBUG(std::cout << "[SERVER] Destroying struct.\n";);
}
		
void	Server::setPort(uint16_t port) {
	_port = port;
}
		
void	Server::setPassword(std::string password) {
	_password = password;
}

uint16_t	Server::getPort() {
	return (_port);
}

std::string Server::getPassword() {
	return (_password);
}


