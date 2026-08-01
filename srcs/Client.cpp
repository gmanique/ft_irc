#include "Client.hpp"

Client::Client() : _fd(-1), _readBuffer("") {
	DEBUG(LOG_TRACE << "[CLIENT] Creating struct with fd `" << _fd << "` and empty buffer.";);
}

Client::Client(int fd) : _fd(fd), _readBuffer("") {
	DEBUG(LOG_TRACE << "[CLIENT] Creating struct with fd `" << _fd << "` and empty buffer.";);
}

Client::Client(int fd, std::string buf) : _fd(fd), _readBuffer(buf) {
	DEBUG(LOG_TRACE << "[CLIENT] Creating struct with fd `" << _fd << "` and buffer `" << buf << "`.";);
}

Client::Client(std::string buf) : _fd(-1), _readBuffer(buf) {
	DEBUG(LOG_TRACE << "[CLIENT] Creating struct with fd `" << _fd << "` and buffer `" << buf << "`.";);
}

Client::~Client() {
	DEBUG(LOG_TRACE << "[CLIENT] Destroying struct.";);
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

