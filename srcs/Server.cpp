#include "Server.hpp"

static void	safe_send(Client &client, std::string to_send, std::string log_err = "")
{
	if (send(client.getFd(), to_send.c_str(), to_send.size(), 0) < 0)
		LOG_ERR << "Send failed.";
	if (!log_err.empty())
		LOG_ERR << log_err;
}

Server::Server() : _port(6667), _password(""), _serverFd(-1) {
	DEBUG(LOG_TRACE << "[SERVER] Creating struct with port `" << _port << "` and password `" << _password << "`.";);
}
Server::Server(int port, const std::string &password) : _port(port), _password(password), _serverFd(-1) {
	DEBUG(LOG_TRACE << "[SERVER] Creating struct with port `" << _port << "` and password `" << _password << "`.";);
}
Server::~Server() {
	if (_serverFd != -1)
		close(_serverFd);
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		close(it->first);
	}
	_clients.clear();
	for (std::map<std::string, Channel *>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
		delete it->second;
	}
	_channels.clear();
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

int	Server::findUser(std::string &username) {
	std::map<int, Client>::const_iterator it;

	for (it = _clients.begin(); it != _clients.end(); ++it) {
		if (it->second.getNickname() == username) {
			return (it->first);
		}
	}
	return (-1);
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

void	Server::do_cap(Client &client, std::vector<std::string> &args) {
	if (args.size() == 2 && args[0] == "LS" && args[1] == "302") {
		LOG_DEBUG << "Sending to client " << client.getFd() << " : " << "CAP * LS :\\r\\n"; 
		SETHASCAP(client.getIsLogged());
		if (send(client.getFd(), "CAP * LS :\r\n", 12, 0) < 0)
		{
			LOG_ERR << "Send failed";
			return ;
		}
	}
}

void	Server::ping_pong(Client &client, std::vector<std::string> &args) {
	std::string rep = "PONG :127.0.0.1\r\n";
	if (send(client.getFd(), rep.c_str(), rep.size(), 0) < 0) {
		LOG_ERR << "Send failed";
		return;
	}
	DEBUG(LOG_DEBUG << "Answered the ping with pong !";);
	(void)args;
}

int	Server::checker_password(Client &client,std::vector<int> &fdsToClose, std::vector<std::string> &args)
{
	std::string nick = client.getNickname().empty() ? "*" : client.getNickname();

	if (HASPASSWORD(client.getIsLogged())) {
		safe_send(client, ":127.0.0.1 462 " + nick + " :Unauthorized command (already registered)\r\n");
		return (-1);
	}

	if (args.empty()) {
		safe_send(client, ":127.0.0.1 461 " + nick + " PASS :Not enough parameters\r\n");
		return (-1);
	}

	if (args[0] != _password) {
		LOG_USER_INFO(client.getNickname()) << "Password is incorrect for client " << client.getFd();
		safe_send(client, ":127.0.0.1 464 " + nick + " :Password incorrect\r\n");
		fdsToClose.push_back(client.getFd());
		return (-1);
	}

	LOG_USER_INFO(client.getNickname()) << "Password is correct for client " << client.getFd();
	SETHASPASSWORD(client.getIsLogged());

	if (ISLOGGED(client.getIsLogged())) {
		std::string rep = ":127.0.0.1 001 " + client.getNickname() + " :Welcome to the Localnet IRC Network " + client.getNickname() + "!" + client.getUser().username + "@127.0.0.1\r\n";
		safe_send(client, rep);
	}

	return (0);
}

int Server::parseNickname(std::string &nickname)
{
	if (nickname.size() > 9 || !std::isalpha(nickname[0])) {
		LOG_USER_INFO(nickname) << "Invalid nickname";
		return (0);
	}
	std::string allowed_chars = "[]{}\\|-_^abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	size_t index = nickname.find_first_not_of(allowed_chars);
	if (index != std::string::npos){
		LOG_USER_INFO(nickname) << "Invalid nickname";
		return (0);
	}
	return (1);
}

int Server::nickname(Client &client, std::vector<std::string> &args)
{
	std::string nick = client.getNickname().empty() ? "*" : client.getNickname();
	std::string msg = "";
	if (args.size() == 0) {
		safe_send(client, ":127.0.0.1 431 " + nick + " NICK :No nickname given\r\n");
		return (-1);
	}
	if (args[0].empty()) {
		safe_send(client, ":127.0.0.1 431 " + nick + " NICK :No nickname given\r\n");
		return (-1);
	}
	if (!HASNICKNAME(client.getIsLogged())) {
		std::string nickname = args[0];
		std::string old_nick = client.getNickname();
	
		if (parseNickname(nickname) == 0)
		{
			safe_send(client, ":127.0.0.1 432 " + nick + " NICK :Erroneous nickname\r\n");
			return (0);
		}
		if (findUser(nickname) != -1)
		{
			safe_send(client, ":127.0.0.1 433 " + nick + nickname + " NICK :Nickname is already in use\r\n");
			LOG_USER_INFO(nickname) << "Already used";
			return (0);
		}
		client.setNickname(nickname);
		LOG_USER_INFO(client.getNickname()) << "Nickname updated.";
		msg = ":" + old_nick + "!" + client.getUser().username + "@127.0.0.1 NICK :" + nickname + "\r\n";
		send(client.getFd(), msg.c_str(), msg.size(), 0);
		LOG_USER_INFO(client.getNickname()) << "Nickname created.";
		SETHASNICKNAME(client.getIsLogged());
		if (ISLOGGED(client.getIsLogged())) {
			std::string rep = ":server_name 001 " + client.getNickname() + " :Welcome to the Localnet IRC Network " + client.getNickname() + "!" + client.getUser().username + "@127.0.0.1\r\n";
			send(client.getFd(), rep.c_str(), rep.size(), 0);
		}
	}
	else {
		std::string old_nick = client.getNickname();
		std::string nickname = args[0];
	
		if (parseNickname(nickname) == 0)
		{
			safe_send(client, ":127.0.0.1 432 " + nick + " NICK :Erroneous nickname\r\n");
			return (0);
		}
		if (findUser(nickname) != -1)
		{
			LOG_USER_INFO(nickname) << "Already used";
			safe_send(client, ":127.0.0.1 433 " + nick + nickname + " NICK :Nickname is already in use\r\n");
			return (0);
		}
		client.setNickname(nickname);
		LOG_USER_INFO(client.getNickname()) << "Nickname updated.";
		msg = ":" + old_nick + "!" + client.getUser().username + "@127.0.0.1 NICK :" + nickname + "\r\n";
		send(client.getFd(), msg.c_str(), msg.size(), 0);
	}	
	return (0);
}

int Server::quit(Client &client, std::vector<int> &fdsToClose, std::vector<std::string> &args)
{
	std::string nick = client.getNickname().empty() ? "*" : client.getNickname();
	std::string reason = "Client Quit";

	if (!args.empty()) {
		reason = "";
		for (size_t i = 0; i < args.size(); ++i) {
			reason += args[i];
			if (i < args.size() - 1)
				reason += " ";
		}
	}
	std::string errMsg = "ERROR :Closing Link: " + nick + " (Quit: " + reason + ")\r\n";
	send(client.getFd(), errMsg.c_str(), errMsg.size(), 0);

	std::string quitMsg = ":" + nick + "!" + client.getUser().username + "@127.0.0.1 QUIT :" + reason + "\r\n";
	std::set<int> notifiedFds;
	std::map<std::string, Channel*>::iterator it = _channels.begin();
	for (; it != _channels.end(); ++it) {
		Channel *c = it->second;
		if (c->hasMember(client.getFd())) {
			std::map<int, Client*> members = c->getMembers();
			for (std::map<int, Client*>::iterator mIt = members.begin(); mIt != members.end(); ++mIt) {
				int targetFd = mIt->first;
				if (targetFd != client.getFd() && notifiedFds.find(targetFd) == notifiedFds.end()) {
					send(targetFd, quitMsg.c_str(), quitMsg.size(), 0);
					notifiedFds.insert(targetFd);
				}
			}
		}
	}

	LOG_USER_INFO(nick) << "Disconnected because: " << reason;
	fdsToClose.push_back(client.getFd());

	return (SUCCESS);
}

int Server::user(Client &client, std::vector<std::string> &args)
{
	
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
	user.realname = args[3];
	client.setUser(user);

	LOG_USER_INFO(client.getNickname()) << "User updated.";
	SETHASUSER(client.getIsLogged());
	if (ISLOGGED(client.getIsLogged())) {
		std::string rep = ":server_name 001 " + client.getNickname() + " :Welcome to the Localnet IRC Network " + client.getNickname() + "!" + client.getUser().username + "@127.0.0.1\r\n";
		send(client.getFd(), rep.c_str(), rep.size(), 0);
	}
	return (0);
}


int Server::do_join(Client &client, std::vector<std::string> &args) {
	std::string nick = client.getNickname().empty() ? "*" : client.getNickname();

	if (args.empty()) {
		safe_send(client, ":127.0.0.1 461 " + nick + " JOIN :Not enough parameters\r\n");
		return (-1);
	}

	std::string channelName = args[0];
	std::string key = (args.size() > 1) ? args[1] : "";

	Channel *c = getChannel(channelName);

	if (!c) {
		c = new Channel(channelName);
		addChannel(c);
	} else {
		if (c->hasMember(client.getFd()))
			return (0);

		if (c->getInviteOnly() && !c->hasInvMember(client.getFd())) {
			safe_send(client, ":127.0.0.1 473 " + nick + " " + channelName + " :Cannot join channel (+i)\r\n");
			return (-1);
		}
		if (!c->getKey().empty() && c->getKey() != key) {
			safe_send(client, ":127.0.0.1 475 " + nick + " " + channelName + " :Cannot join channel (+k)\r\n");
			return (-1);
		}
		if (c->getUserLimit() > 0 && c->getMembers().size() >= c->getUserLimit()) {
			safe_send(client, ":127.0.0.1 471 " + nick + " " + channelName + " :Cannot join channel (+l)\r\n");
			return (-1);
		}
	}

	c->addMember(&client);
	if (c->hasInvMember(client.getFd()))
		c->removeInvMember(client.getFd());

	std::string joinMsg = ":" + nick + "!" + client.getUser().username + "@127.0.0.1 JOIN " + channelName + "\r\n";
	std::map<int, Client*> members = c->getMembers();
	for (std::map<int, Client*>::iterator it = members.begin(); it != members.end(); ++it) {
		send(it->first, joinMsg.c_str(), joinMsg.size(), 0);
	}

	if (!c->getTopic().empty()) {
		safe_send(client, ":127.0.0.1 332 " + nick + " " + channelName + " :" + c->getTopic() + "\r\n");
	} else {
		safe_send(client, ":127.0.0.1 331 " + nick + " " + channelName + " :No topic is set\r\n");
	}

	std::string names = "";
	for (std::map<int, Client*>::iterator it = members.begin(); it != members.end(); ++it) {
		if (!names.empty())
			names += " ";
		if (c->isOperator(it->first))
			names += "@";
		names += it->second->getNickname();
	}
	safe_send(client, ":127.0.0.1 353 " + nick + " = " + channelName + " :" + names + "\r\n");

	safe_send(client, ":127.0.0.1 366 " + nick + " " + channelName + " :End of /NAMES list\r\n");

	return (SUCCESS);
}

int Server::send_to_channel(Client &client, std::vector<std::string> &args) {
	std::string channel_name = args[0];
	std::string nick = client.getNickname().empty() ? "*" : client.getNickname();

	Channel *c = getChannel(channel_name);
	if (!c) {
		std::string msg = ":127.0.0.1 403 " + nick + " " + channel_name + " :No such channel\r\n";
		send(client.getFd(), msg.c_str(), msg.size(), 0);
		LOG_USER_ERR(client.getNickname()) << "Channel " << channel_name << " does not exist."; 
		return (-1);
	}

	if (!c->hasMember(client.getFd())) {
		std::string msg = ":127.0.0.1 404 " + nick + " " + channel_name + " :Cannot send to channel\r\n";
		send(client.getFd(), msg.c_str(), msg.size(), 0);
		LOG_USER_ERR(client.getNickname()) << "Cannot send to channel " << channel_name << " (not a member).";
		return (-1);
	}

	std::string message = "";
	for (size_t i = 1; i < args.size(); i++) {
		message += args[i];
		if (i < args.size() - 1)
			message += " ";
	}

	int sender = client.getFd();
	std::map<int, Client *> clients = c->getMembers();
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		int clientFd = it->first;
		if (clientFd == sender)
			continue;
		std::string msg = ":" + client.getNickname() + "!" + client.getUser().username 
			+ "@127.0.0.1 PRIVMSG " + channel_name + " :" + message + "\r\n";
		send(clientFd, msg.c_str(), msg.size(), 0);
	}
	return (0);
}

int Server::send_to_user(Client &client, std::vector<std::string> &args) {
	std::string destination_name = args[0];
	int fd = findUser(destination_name);

	if (fd == -1) {
		std::string nick = client.getNickname().empty() ? "*" : client.getNickname();
		std::string msg = ":127.0.0.1 401 " + nick + " " + destination_name + " :No such nick/channel\r\n";
		send(client.getFd(), msg.c_str(), msg.size(), 0);
		LOG_USER_ERR(client.getNickname()) << "User " << destination_name << " does not exist."; 
		return (-1);
	}

	std::string message = "";
	for (size_t i = 1; i < args.size(); i++) {
		message += args[i];
		if (i < args.size() - 1)
			message += " ";
	}

	std::string msg = ":" + client.getNickname() + "!" + client.getUser().username + "@127.0.0.1 PRIVMSG " + destination_name + " :" + message + "\r\n";
	send(fd, msg.c_str(), msg.size(), 0);
	return (0);
}

void Server::do_msg(Client &client, std::vector<std::string> &args) {
	std::string nick = client.getNickname().empty() ? "*" : client.getNickname();

	if (args.empty()) {
		std::string msg = ":127.0.0.1 411 " + nick + " :No recipient given (PRIVMSG)\r\n";
		send(client.getFd(), msg.c_str(), msg.size(), 0);
		return;
	}

	if (args.size() < 2 || args[1].empty()) {
		std::string msg = ":127.0.0.1 412 " + nick + " :No text to send\r\n";
		send(client.getFd(), msg.c_str(), msg.size(), 0);
		return;
	}

	DEBUG(LOG_USER_DEBUG(client.getNickname()) << "Sending message to " << args[0];);

	if (args[0][0] == '#') {
		send_to_channel(client, args);
	} else {
		send_to_user(client, args);
	}
}


void	Server::do_mode(Client &client, std::vector<std::string> &args)
{
	DEBUG(LOG_DEBUG << "Entering Server::do_mode");

	if (!ISLOGGED(client.getIsLogged())) {
		safe_send(client, ":127.0.0.1 451 * :You have not registered\r\n", "Client not logged");
		return;
	}
	if (args.size() < 2)
	{
		safe_send(client, ":127.0.0.1 461 " + client.getNickname() + " MODE :Not enough parameters\r\n", "Mode: not enough parameters");
		return ;
	}
	if (args[0] == client.getNickname())
	{
		LOG_DEBUG << "Mode not on channel";
		return ;
	}
	

	std::string channelName = args[0];
	std::string modeString = args[1];
	Channel 	*channel = getChannel(channelName);
	if (!channel) {
		safe_send(client, "127.0.0.1 403 " + client.getNickname() + " " + channelName + " :No such channel\r\n", channelName + " : no such channel");
		return;
	}
	if (!channel->hasMember(client.getFd())) {
		safe_send(client, "127.0.0.1 442 " + client.getNickname() + " " + channelName + " :You're not on that channel\r\n", client.getNickname() + " : not on channel " + channelName);
		return;
	}
	if (!channel->isOperator(client.getFd())) {
		safe_send(client, "127.0.0.1 482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n", client.getNickname() + " : is not operator on channel " + channelName);
		return;
	}

	size_t	args_i = 1;
	char 	sign;
	if (modeString[0] == '+' || modeString[0] == '-')
		sign = modeString[0];
	else {
		LOG_ERR << "Mode: invalid modestring";
		return;
	}

	for (size_t i = 1; i < modeString.size(); i++) {
		char c = modeString[i];
		if (c == '+' || c == '-') {
			sign = c;
			continue;
		}
		switch (c) {
			case 'i': {
				channel->setInviteOnly(sign == '+');
				break;
			}
			case 't': {
				channel->setTopicOpOnly(sign == '+');
				break;
			}
			case 'k': {
				if (sign == '-')
					channel->removeKey();
				else if (++args_i >= args.size())
					safe_send(client, "127.0.0.1 461 " + client.getNickname() + " MODE :Not enough parameters\r\n", "Mode: setKey: missing argument");
				else
					channel->setKey(args[args_i]);
				break;
			}
			case 'o': {
				if (++args_i >= args.size()) {
					safe_send(client, "127.0.0.1 461 " + client.getNickname() + " MODE :Not enough parameters\r\n", "Mode: addOp/removeOp: missing argument");
					break;
				}
				int client_fd = findUser(args[args_i]);
				if (client_fd == -1) {
					safe_send(client, "127.0.0.1 441 " + client.getNickname() + " " + channelName + " :They aren't on that channel\r\n", "Mode: addOp/removeOp: bad argument");
					break;
				}
				if (sign == '+')
					channel->addOperator(client_fd);
				else
					channel->removeOperator(client_fd);
				break;
			}
			case 'l': {
				if (sign == '-')
					channel->setUserLimit(0);
				else {
					if (++args_i >= args.size()) {
						safe_send(client, "127.0.0.1 461 " + client.getNickname() + " MODE :Not enough parameters\r\n", "Mode: setUserLimit: missing argument");
						break;
					}
					std::istringstream iss(args[args_i]);
					long n;
					iss >> n;
					if (iss.fail() || n <= 0) {
						LOG_ERR << "Mode: setUserLimit: bad argument";
						break;
					}
					channel->setUserLimit(n);
				}
				break;
			}
			default: {
				safe_send(client, "127.0.0.1 472 " + client.getNickname() + " " + std::string(1, c) + " :is unknown mode char to me\r\n", "Mode: unknown argument \'" + std::string(1, c) + "\' in modestring");
				break;
			}
		}
	}
	std::string str = client.getNickname() + "!" + client.getUser().username + "@127.0.0.1 MODE " + channelName + " " + modeString;
	for (size_t i = 2; i < args.size(); i++)
		str += " " + args[i];
	str += "\r\n";
	std::map<int, Client *> members = channel->getMembers();
	for(std::map<int, Client *>::iterator it = members.begin(); it != members.end(); it++) {
		int currfd = it->first;
		if (send(currfd, str.c_str(), str.size(), 0) < 0) {
			LOG_ERR << "Send failed.";
			return;
		}
	}
}

int Server::do_invite(Client &client, std::vector<std::string> &args) {
	std::string nick = client.getNickname().empty() ? "*" : client.getNickname();
	std::string msg;

	if (args.size() < 2) {
		LOG_ERR << "Not enough arguments.";
		msg = ":127.0.0.1 461 " + nick + " INVITE :Not enough parameters\r\n";
		send(client.getFd(), msg.c_str(), msg.size(), 0);
		return (-1);
	}

	std::string user = args[0];
	std::string channel = args[1];

	Channel *c = getChannel(channel);
	if (!c) {
		LOG_ERR << "Channel does not exist.";
		msg = ":127.0.0.1 403 " + nick + " " + channel + " :No such channel\r\n";
		send(client.getFd(), msg.c_str(), msg.size(), 0);
		return (-1);
	}

	if (!c->hasMember(client.getFd())) {
		LOG_USER_ERR(client.getNickname()) << "You must be part of the channel to invite someone.";
		msg = ":127.0.0.1 442 " + nick + " " + channel + " :You're not on that channel\r\n";
		send(client.getFd(), msg.c_str(), msg.size(), 0);
		return (-1);
	}

	if (c->getInviteOnly() && !c->isOperator(client.getFd())) {
		LOG_USER_ERR(client.getNickname()) << "You must be operator to invite someone.";
		msg = ":127.0.0.1 482 " + nick + " " + channel + " :You're not channel operator\r\n";
		send(client.getFd(), msg.c_str(), msg.size(), 0);
		return (-1);
	}

	int dest_fd = findUser(user);
	if (dest_fd == -1) {
		LOG_ERR << "No such user as " << user << ".";
		msg = ":127.0.0.1 401 " + nick + " " + user + " :No such nick/channel\r\n";
		send(client.getFd(), msg.c_str(), msg.size(), 0);
		return (-1);
	}

	if (c->hasMember(dest_fd)) {
		LOG_ERR << user << " is already part of the channel.";
		msg = ":127.0.0.1 443 " + nick + " " + user + " " + channel + " :is already on channel\r\n";
		send(client.getFd(), msg.c_str(), msg.size(), 0);
		return (-1);
	}

	c->addInvMember(dest_fd);

	msg = ":127.0.0.1 341 " + nick + " " + user + " " + channel + "\r\n";
	send(client.getFd(), msg.c_str(), msg.size(), 0);

	msg = ":" + nick + "!" + client.getUser().username + "@127.0.0.1 INVITE " + user + " :" + channel + "\r\n";
	send(dest_fd, msg.c_str(), msg.size(), 0);

	return (SUCCESS);
}

int Server::do_part(Client &client, std::vector<std::string> &args) {
	std::string nick = client.getNickname().empty() ? "*" : client.getNickname();
	if (args.empty()) {
		safe_send(client, ":127.0.0.1 461 " + nick + " PART :Not enough parameters\r\n");
		return (-1);
	}
	std::string channelName = args[0];
	Channel *c = getChannel(channelName);
	if (!c) {
		safe_send(client, ":127.0.0.1 403 " + nick + " " + channelName + " :No such channel\r\n");
		return (-1);
	}
	if (!c->hasMember(client.getFd())) {
		safe_send(client, ":127.0.0.1 442 " + nick + " " + channelName + " :You're not on that channel\r\n");
		return (-1);
	}
	std::string reason = "";
	if (args.size() > 1) {
		for (size_t i = 1; i < args.size(); ++i) {
			reason += args[i];
			if (i < args.size() - 1)
				reason += " ";
		}
	} else {
		reason = nick;
	}
	std::string partMsg = ":" + nick + "!" + client.getUser().username + "@127.0.0.1 PART " + channelName + " :" + reason + "\r\n";
	std::map<int, Client*> members = c->getMembers();
	for (std::map<int, Client*>::iterator it = members.begin(); it != members.end(); ++it) {
		send(it->first, partMsg.c_str(), partMsg.size(), 0);
	}
	unlinkClientFromChannel(&client, channelName);
	return (SUCCESS);
}

int Server::do_kick(Client &client, std::vector<std::string> &args) {
	std::string nick = client.getNickname().empty() ? "*" : client.getNickname();
	if (args.size() < 2) {
		safe_send(client, ":127.0.0.1 461 " + nick + " KICK :Not enough parameters\r\n");
		return (USAGE_ERROR);
	}
	std::string channelName = args[0];
	Channel *c = getChannel(channelName);
	
	if (!c) {
		safe_send(client, ":127.0.0.1 403 " + nick + " " + channelName + " :No such channel\r\n");
		return (USAGE_ERROR);
	}
	int	dest = findUser(args[1]);
	if (dest == -1) {
		safe_send(client, ":127.0.0.1 401 " + nick + " " + args[1] + " :No such nick/channel\r\n");
		return (USAGE_ERROR);
	}
	if (!c->hasMember(client.getFd())) {
		safe_send(client, ":127.0.0.1 442 " + nick + " " + channelName + " :You're not on that channel\r\n");
		return (USAGE_ERROR);
	}
	if (!c->isOperator(client.getFd())) {
		LOG_USER_ERR(client.getNickname()) << "You must be operator to invite someone.";
		std::string msg = ":127.0.0.1 482 " + nick + " " + channelName + " :You're not channel operator\r\n";
		send(client.getFd(), msg.c_str(), msg.size(), 0);
		return (USAGE_ERROR);
	}
	if (!c->hasMember(dest)) {
		safe_send(client, ":127.0.0.1 441 " + nick + " " + args[1] + " " + channelName + " :They aren't on that channel\r\n");
		return (USAGE_ERROR);
	}
	std::string reason = "";
	if (args.size() > 2) {
		for (size_t i = 2; i < args.size(); ++i) {
			reason += args[i];
			if (i < args.size() - 1)
				reason += " ";
		}
	} else {
		reason = nick;
	}
	std::string kickMsg = ":" + nick + "!" + client.getUser().username + "@127.0.0.1 KICK " + channelName + " " + args[1] + " :" + reason + "\r\n";

	std::map<int, Client*> members = c->getMembers();
	for (std::map<int, Client*>::iterator it = members.begin(); it != members.end(); ++it) {
		send(it->first, kickMsg.c_str(), kickMsg.size(), 0);
	}
	Client *cDest = c->getMember(dest);
	unlinkClientFromChannel(cDest, channelName);
	return (SUCCESS);
}

int	Server::do_topic(Client &client, std::vector<std::string> &args) {
std::string nick = client.getNickname().empty() ? "*" : client.getNickname();
	if (args.empty()) {
		safe_send(client, ":127.0.0.1 461 " + nick + " TOPIC :Not enough parameters\r\n");
		return (USAGE_ERROR);
	}
	std::string channelName = args[0];
	Channel *c = getChannel(channelName);
	if (!c) {
		safe_send(client, ":127.0.0.1 403 " + nick + " " + channelName + " :No such channel\r\n");
		return (USAGE_ERROR);
	}
	if (!c->hasMember(client.getFd())) {
		safe_send(client, ":127.0.0.1 442 " + nick + " " + channelName + " :You're not on that channel\r\n");
		return (USAGE_ERROR);
	}
	if (args.size() == 1) {
		std::string topic = c->getTopic();
		if (topic.empty()){
			safe_send(client, ":127.0.0.1 331 " + nick + " " + channelName + " :No topic is set\r\n");
		}
		else {
			safe_send(client, ":127.0.0.1 332 " + nick + " " + channelName + " :" + c->getTopic() + "\r\n");
		}
		return (SUCCESS);
	}
	if (c->getTopicOpOnly() && !c->isOperator(client.getFd())) {
		safe_send(client, ":127.0.0.1 482 " + nick + " " + channelName + " :You're not channel operator\r\n");
		return (USAGE_ERROR);
	}
	std::string newTopic = "";
	for(size_t i = 1; i < args.size(); i++) {
		newTopic += args[i];
		if (i < args.size()-1) {
			newTopic += " ";
		}
	}
	c->setTopic(newTopic);
	std::string topMsg = ":" + nick + "!" + client.getUser().username + "@127.0.0.1 TOPIC " + channelName + " :" + newTopic + "\r\n"; 
	std::map<int, Client*> members = c->getMembers();
	for (std::map<int, Client*>::iterator it = members.begin(); it != members.end(); ++it) {
		send(it->first, topMsg.c_str(), topMsg.size(), 0);
	}
	return (SUCCESS);
}

int	Server::executeCommand(Client &client, std::string &command, std::vector<int> &fdsToClose)
{
	std::string cmd = "";
	std::vector<std::string> args;
	parseCommand(command, cmd, args);
	if (cmd == "CAP")
		Server::do_cap(client, args);
	else if (cmd == "PASS") {
		if (checker_password(client, fdsToClose, args) == -1)
			return (-1);
	}
	else if (cmd == "NICK") {
		nickname(client, args);
	}
	else if (cmd == "USER") {
		user(client, args);
	}
	else if (!ISLOGGED(client.getIsLogged())) {
		safe_send(client, ":127.0.0.1 451 * :You have not registered\r\n", "Client not logged");
		LOG_ERR << "Please connect before anything.";
		return (-1);
	}
	else if (cmd == "JOIN") {
		do_join(client, args);
	}
	else if (cmd == "PING")
		ping_pong(client, args);
	else if (cmd == "PRIVMSG") {
		do_msg(client, args);
	}
	else if (cmd == "MODE")
		do_mode(client, args);
	else if (cmd == "INVITE") {
		do_invite(client, args);
	}
	else if (cmd == "TOPIC") {
		do_topic(client, args);
	}
	else if (cmd == "KICK") {
		do_kick(client, args);
	}
	else if (cmd == "PART") {
		do_part(client, args);
	}
	else if (cmd == "QUIT")
		quit(client, fdsToClose, args);
	return (0);
}


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

static void handler_sig(int) {}

static int handle_signal()
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

	if (handle_signal() == MEMORY_ERROR)
		return (MEMORY_ERROR);
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
