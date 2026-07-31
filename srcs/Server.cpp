#include "Server.hpp"

Server::Server() : _port(6667), _password("") {
	DEBUG(std::cout << "[SERVER] Creating struct with port `" << _port << "` and password `" << _password << "`.\n";);
}

Server::Server(int port, const std::string &password) : _port(port), _password(password) {
	DEBUG(std::cout << "[SERVER] Creating struct with port `" << _port << "` and password `" << _password << "`.\n";);
}

Server::~Server() {
	DEBUG(std::cout << "[SERVER] Destroying struct.\n";);
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



// verifier les erreurs jai la flemme la
void	Server::init() {
	int	fd = socket(AF_INET, SOCK_STREAM, 0);
	
	int	opt = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	fcntl(fd, F_SETFL, O_NONBLOCK);

	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(_port);
	bind(fd, (struct sockaddr*)&addr, sizeof(addr));

	listen(fd, SOMAXCONN);
}

void	Server::run() {
	return ;
}
