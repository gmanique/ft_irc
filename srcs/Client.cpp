#include "Client.hpp"

Client::Client() : _fd(-1), _readBuffer("") {
	DEBUG(std::cout << "[CLIENT] Creating struct with fd `" << _fd << "` and empty buffer.\n";);
}

Client::Client(int fd) : _fd(fd), _readBuffer("") {
	DEBUG(std::cout << "[CLIENT] Creating struct with fd `" << _fd << "` and empty buffer.\n";);
}

Client::Client(int fd, std::string buf) : _fd(fd), _readBuffer(buf) {
	DEBUG(std::cout << "[CLIENT] Creating struct with fd `" << _fd << "` and buffer `" << buf << "`.\n";);
}

Client::Client(std::string buf) : _fd(-1), _readBuffer(buf) {
	DEBUG(std::cout << "[CLIENT] Creating struct with fd `" << _fd << "` and buffer `" << buf << "`.\n";);
}

Client::~Client() {
	DEBUG(std::cout << "[CLIENT] Destroying struct.\n";);
}
		
void		Client::setFd(int fd) {
	_fd = fd;
}
		
void		Client::setBuffer(std::string buf) {
	_readBuffer = buf;
}

int			&Client::getFd() {
	return (_fd);
}

std::string &Client::getBuffer() {
	return (_readBuffer);
}

void	Client::appendBuffer(std::string buf) {
	_readBuffer += buf;
}

