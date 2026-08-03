#ifndef SERVER_HPP
# define SERVER_HPP

#include "ft_irc.hpp"

class Client;
class Channel;

class Server {
	private:
		int									_port; // Port d'ecoute (entre 6665 et 6669)
		std::string							_password;
		int									_serverFd;
		std::vector<pollfd>					_pollFds;
		std::map<int, Client>				_clients; 

		std::map<std::string, Channel *>	_channels;
		uint8_t								_running;
		
		void								acceptNewClient();
		void								handleClientData(int clientFd, std::vector<int> &fdsToClose);
		void								disconnectClient(int fd);
		
		int									executeCommand(Client &client, std::string &command);
		void								linkClientToChannel(Client *client, std::string &channel_name);
		void								unlinkClientFromChannel(Client *client, std::string &channel_name);

	public:
		Server();
		Server(int port, const std::string& password);
		~Server();

		void								setPort(int port);
		void								setPassword(const std::string& password);
		int									getPort() const;
		const std::string&					getPassword() const;

		Channel								*getChannel(std::string &name) const;
		void								addChannel(Channel *channel);
		void								deleteChannel(std::string &name);

		
		int									init();
		int									run();

};

#endif
