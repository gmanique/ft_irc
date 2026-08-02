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
const std::string	&Server::getPassword() const {
	return (_password);
}
Channel				*Server::getChannel(std::string &name) const {
	return (_channels.at(name));
}
void			Server::addChannel(Channel *channel) {
	std::string name = channel->getName();
	_channels.insert(std::make_pair(name, channel));
}
void			Server::deleteChannel(std::string &name) {
	_channels.erase(name);
}

void	Server::linkClientToChannel(Client *client, std::string &channel_name) {
	Channel *c = getChannel(channel_name);
	if (!c) {
		LOG_INFO << "creating channel `" << channel_name << "`.";
		c = new Channel(channel_name); // Deja gere par le try catch du main
		addChannel(c);
	}
	c->addMember(client);
}

void	Server::unlinkClientFromChannel(Client *client, std::string &channel_name) {
	Channel *c = getChannel(channel_name);
	if (!c)
		return ;
	if (!c->hasMember(client->getFd()))
		return ;
	c->removeMember(client->getFd());
	if (c->getMembers().empty()) {
		deleteChannel(channel_name);
		delete(c);
	}
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

void	Server::acceptNewClient() {
	DEBUG(LOG_DEBUG << "Entering Server::acceptNewClient";);
	sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);

	LOG_TRACE << "Trying to connect new client";
	int clientFd = accept(_serverFd, (struct sockaddr*)&clientAddr, &clientLen);
	if (clientFd < 0) {
		LOG_ERR << "Client could not be accepted for an unknown reason";
		return;
	}

	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0) {
		LOG_ERR << "Could not activate non blocking mode.";
		close(clientFd);
		return;
	}

	pollfd pfd;
	pfd.fd = clientFd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_pollFds.push_back(pfd);
	
	try {
		_clients[clientFd] = Client(clientFd);
	} catch(const std::exception &e) {
		LOG_ERR << "Client creation failed : " << e.what();
		_pollFds.pop_back();
		close(clientFd);
		return;
	}
	LOG_INFO << "New client connected on " << clientFd;
}


// A faire
void	Server::handleClientData(int clientFd, std::vector<int> &fdsToClose) {
	DEBUG(LOG_DEBUG << "Entering Server::handleClientData";);

	char buffer[MAX_MSG_SIZE];
	std::memset(buffer, 0, MAX_MSG_SIZE);

	ssize_t	bytesRead = recv(clientFd, buffer, MAX_MSG_SIZE-1, 0);
	if (bytesRead <= 0) {
		if (bytesRead == 0) {
			LOG_INFO << "Client on fd " << clientFd << " disconnected (EOF).";
		} else {
			LOG_ERR << "Could not read (recv) from fd " << clientFd;
		}
		fdsToClose.push_back(clientFd);
		return;
	}

	Client &client = _clients[clientFd];
	client.appendBuffer(std::string(buffer, bytesRead));

	std::string command;
	while (client.extractCommand(command)) {
		
		DEBUG(LOG_INFO << "Command received from client " << clientFd << " : " << command;);
		client.executeCommand(client, command);
	}
}

void	Server::disconnectClient(int fd) {
	DEBUG(LOG_DEBUG << "Entering Server::disconnectClient";);
	close(fd);
	for (std::vector<pollfd>::iterator it = _pollFds.begin(); it != _pollFds.end(); ++it) {
		if (it->fd == fd) {
			_pollFds.erase(it);
			LOG_INFO << "Client with fd " << fd << " got disconnected.";
			return;
		}
	}
	LOG_INFO << "Couldn't find client " << fd;
}

int	Server::run() {
	LOG_INFO << "Server starting..";
	_running = 1;
	while(_running) {
		int	ret = poll(&_pollFds[0], _pollFds.size(), -1);
		if (ret < 0) {
			// Probablement gerer les signaux ici, apparament poll peut s'arreter a cause d'un signal et c'est ok, dans ce cas faire continue;
			LOG_ERR << "Poll critical failure.";
			return (MEMORY_ERROR);
		}
		std::vector<int>	fdsToClose;
		
		DEBUG(LOG_DEBUG << "Got out of the poll";);

		for(size_t i = 0; i < _pollFds.size(); i++) {
			if (_pollFds[i].revents & POLLIN) {
				if (_pollFds[i].fd == _serverFd) {
					acceptNewClient();
				} else {
					handleClientData(_pollFds[i].fd, fdsToClose);
				}
			}
		}

		for(size_t i = 0; i < fdsToClose.size(); i++) {
			disconnectClient(fdsToClose[i]);
		}
	}
	return (SUCCESS);
}
