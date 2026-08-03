#include "Client.hpp"

Client::Client() : _fd(-1), _readBuffer("") {
	DEBUG(LOG_TRACE << "[CLIENT] Creating struct with default fd `" << _fd << "` and empty buffer.";);
}

Client::Client(int fd) : _fd(fd), _readBuffer("") {
	DEBUG(LOG_TRACE << "[CLIENT] Creating struct with fd `" << _fd << "` and empty buffer.";);
}

Client::Client(int fd, std::string buf) : _fd(fd), _readBuffer(buf) {
	DEBUG(LOG_TRACE << "[CLIENT] Creating struct with fd `" << _fd << "` and buffer `" << buf << "`.";);
}

Client::Client(std::string buf) : _fd(-1), _readBuffer(buf) {
	DEBUG(LOG_TRACE << "[CLIENT] Creating struct with default fd `" << _fd << "` and buffer `" << buf << "`.";);
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

int			Client::getFd() const {
	return (_fd);
}

const std::string &Client::getBuffer() const {
	return (_readBuffer);
}

const std::string&		Client::getNickname() const {
	return (_nickname);
}

void			Client::setNickname(std::string &nickname) {
	_nickname = nickname;
}

const std::string&		Client::getUser() const {
	return (_user);
}

void			Client::setUser(std::string &user) {
	_user = user;
}

void	Client::appendBuffer(std::string buf) {
	_readBuffer += buf;
}

uint8_t	Client::extractCommand(std::string &command) {
	size_t pos = _readBuffer.find("\n");
	if (pos == std::string::npos)
		return (0);

	command = _readBuffer.substr(0, pos);

	if (!command.empty() && command[command.size() - 1] == '\r') {
		command.erase(command.size() - 1);
	}

	_readBuffer.erase(0, pos + 1);
	return (1);
}




