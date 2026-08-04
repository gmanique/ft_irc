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
	try {
		return (_channels.at(name));
	} catch (const std::exception &e) {
		return (NULL);
	}
}
void			Server::addChannel(Channel *channel) {
	std::string name = channel->getName();
	_channels.insert(std::make_pair(name, channel));
}
void			Server::deleteChannel(std::string &name) {
	_channels.erase(name);
}

// return : 0 = a rejoint, 1 : etait deja dedans
int	Server::linkClientToChannel(Client *client, std::string &channel_name) {
	Channel *c = getChannel(channel_name);
	if (!c) {
		LOG_INFO << "creating channel `" << channel_name << "`.";
		c = new Channel(channel_name); // Deja gere par le try catch du main
		addChannel(c);
		LOG_USER_INFO(channel_name) << " Channel created.";
	}
	if (!c->hasMember(client->getFd())) {
		c->addMember(client);
		LOG_USER_INFO(client->getNickname()) << " added to channel " << channel_name << ".";
		return (0);
	}
	LOG_USER_INFO(client->getNickname()) << " already part of channel " << channel_name << ".";
	return (1);
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

void parseCommand(const std::string &line, std::string &command, std::vector<std::string> &args) {
    std::stringstream ss(line);
    std::string token;

    if (!(ss >> command))
        return;

    while (ss >> token) {
        if (token[0] == ':') {
            std::string trailing;
            std::getline(ss, trailing);
            
            std::string fullTrailing = token.substr(1) + trailing;
            args.push_back(fullTrailing);
            break;
        }
        args.push_back(token);
    }
}

void	Server::do_cap(Client &client, std::string &cmd) {
	if (cmd == "CAP LS 302") {
		LOG_DEBUG << "Sending to client " << client.getFd() << " : " << "CAP * LS :\\r\\n"; 
		if (send(client.getFd(), "CAP * LS :\r\n", 12, 0) < 0)
		{
			LOG_ERR << "Send failed";
			return ;
		}
	}
}

void	Server::ping_pong(Client &client, std::string &cmd, std::vector<std::string> &args) {
	std::string rep = "PONG :127.0.0.1";
	if (send(client.getFd(), rep.c_str(), rep.size(), 0) < 0) {
		LOG_ERR << "Send failed";
		return;
	}
	DEBUG(LOG_DEBUG << "Answered the ping with pong !";);
	(void)cmd;
	(void)args;
}

int	Server::checker_password(Client &client, std::string &cmd, std::vector<int> &fdsToClose, std::vector<std::string> &args)
{
	if (HASPASSWORD(client.getIsLogged())) {
		LOG_INFO << "You already logged in";
		return (0);
	}
	if (args.size() != 1)
		return (-1);//probleme
	(void)cmd;
	
	if (args[0] == _password) {
		LOG_INFO << "Password is correct";
		SETHASPASSWORD(client.getIsLogged());
	}
	else
	{
		LOG_DEBUG << "Sending to client " << client.getFd() << " : " << ":127.0.0.1 464 * :Password incorrect";
		fdsToClose.push_back(client.getFd());
		if (send(client.getFd(), ":127.0.0.1 464 * :Password incorrect", 36, 0) < 0)
		{
			LOG_ERR << "Send failed";
			return (-1);
		}
		LOG_INFO << "Password is incorrect";
		return (-1);
	}
	return (0);
}

std::string Server::first_word(std::string &command)
{
	size_t pos = command.find(' ');
	if (pos != std::string::npos)
		LOG_PROTO << "Pas d'espace OK"; 
	std::string cmd = command.substr(0, pos);
	return (cmd);
	
}

int Server::nickname(Client &client, std::string &cmd)
{
	std::size_t pos = cmd.find_last_not_of(" \r\n\t\f\v");
	cmd = cmd.substr(0, pos + 1);

	pos = cmd.find_last_of(" \r\n\t\f\v");
	std::string nickname = cmd.substr(pos + 1);

	client.setNickname(nickname);
	LOG_USER_INFO(client.getNickname()) << "Nickname updated.";
	SETHASNICKNAME(client.getIsLogged());
	return (0);
}

int Server::user(Client &client, std::string &cmd, std::vector<std::string> &args)
{
	/*
	// on tronque l'input
	std::size_t pos = cmd.find_last_not_of(" \r\n\t\f\v");
	cmd = cmd.substr(0, pos + 1);
	pos = cmd.find_first_not_of(" \r\n\t\f\v");
	cmd = cmd.substr(pos);
	*/
	(void)cmd;
	if (ISLOGGED(client.getIsLogged())) {
		std::string rep = "462 " + client.getNickname() + " :Unauthorized command (already registered)";
		send(client.getFd(), rep.c_str(), rep.size(), 0);
        LOG_USER_ERR(client.getNickname()) << "Tried to register twice.";
		return (-1);
    }
	if  (args.size() != 4) {
		std::string rep = "461 " + (client.getNickname().empty() ? "*" : client.getNickname()) + " USER :Not enough parameters";
		send(client.getFd(), rep.c_str(), rep.size(), 0);
        return (-1);
    }


	t_user	user;
	user.username = args[0];
	user.realname = args[3]; // faut retirer le ':', jai la flemme la
	client.setUser(user);

	LOG_USER_INFO(client.getNickname()) << "User updated.";
	SETHASUSER(client.getIsLogged());
	std::string rep = ":server_name 001 " + client.getNickname() + " :Welcome to the Localnet IRC Network " + client.getNickname() + "!" + client.getUser().username + "@127.0.0.1\r\n";
	send(client.getFd(), rep.c_str(), rep.size(), 0);
	// envoyer la reponse valide au client je suppose
	return (0);
}


int	Server::do_join(Client &client, std::string &command, std::vector<int> &fdsToClose) {
	if (!ISLOGGED(client.getIsLogged())) {
		LOG_USER_ERR(client.getNickname()) << "Client must be fully logged in before connecting to a channel.";
		DEBUG(LOG_DEBUG << "\nHasPassword : " << HASPASSWORD(client.getIsLogged())						 << "\nHasNickname : " << HASNICKNAME(client.getIsLogged())						  << "\nHasUser : " << HASUSER(client.getIsLogged()););
		return (USAGE_ERROR);
	}
	std::size_t pos = command.find_last_not_of(" \r\n\t\f\v");
	command = command.substr(0, pos + 1);

	pos = command.find_last_of(" \r\n\t\f\v");
	std::string channel = command.substr(pos + 1);

	LOG_USER_TRACE(client.getNickname()) << " trying to connect to channel : " << channel;
	(void)fdsToClose;
	if (!linkClientToChannel(&client, channel)) {
		// renvoyer un message disant JOIN etc au client
		;
	}
	return (0);
}


int	Server::executeCommand(Client &client, std::string &command, std::vector<int> &fdsToClose)
{

	std::string cmd = "";
	std::vector<std::string> args;
	parseCommand(command, cmd, args);
	if (cmd == "CAP")
		Server::do_cap(client, command);
	else if (cmd == "PASS")
	{
		if (checker_password(client, command, fdsToClose, args) == -1)
			return (-1);
	}
	else if (cmd == "NICK")
	{
		nickname(client, command);
	}
	// c pas sure a voir
	else if (cmd == "USER")
	{
		user(client, command, args);
	}
	else if (cmd == "JOIN") {
		do_join(client, command, fdsToClose);
	}
	else if (cmd == "PING")
		ping_pong(client, cmd, args);
	return (0);
}


// A faire
void	Server::handleClientData(int clientFd, std::vector<int> &fdsToClose) {
	DEBUG(LOG_DEBUG << "Entering Server::handleClientData";);

	char buffer[MAX_MSG_SIZE];
	std::memset(buffer, 0, MAX_MSG_SIZE);

	ssize_t	bytesRead = recv(clientFd, buffer, MAX_MSG_SIZE-1, 0);
	if (bytesRead <= 0) {
		if (bytesRead == 0) {
			LOG_USER_INFO(_clients[clientFd].getNickname()) << "Client on fd " << clientFd << " disconnected (EOF).";
		} else {
			LOG_USER_ERR(_clients[clientFd].getNickname()) << "Could not read (recv) from fd " << clientFd;
		}
		fdsToClose.push_back(clientFd);
		return;
	}

	Client &client = _clients[clientFd];
	client.appendBuffer(std::string(buffer, bytesRead));

	std::string command;
	while (client.extractCommand(command)) {
		
		DEBUG(LOG_USER_INFO(client.getNickname()) << "Command received from client " << clientFd << " : " << command;);
		if (executeCommand(client, command, fdsToClose) == -1)
			return;
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

void Server::handler_sig(int)
{
}

int Server::handle_signal()
{
	struct sigaction sa;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = handler_sig;
	if (sigaction(SIGINT, &sa, NULL) == -1)
		return (MEMORY_ERROR);
	if (sigaction(SIGQUIT, &sa, NULL) == -1)
		return (MEMORY_ERROR);
	return (0);
}

int	Server::run() {
	LOG_INFO << "Server starting..";

	handle_signal();
	_running = 1;

	while(_running) {
		int	ret = poll(&_pollFds[0], _pollFds.size(), -1);
		if (ret < 0) {			
			if (errno == EINTR) {
				LOG_INFO << "Poll interrupted by a signal, turning server off..";
				_running = 0;
				continue;
			}
			LOG_ERR << "Poll critical failure.";
			return (MEMORY_ERROR);
		}
		std::vector<int>	fdsToClose;
		
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
