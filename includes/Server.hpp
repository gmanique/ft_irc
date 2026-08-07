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

		int									linkClientToChannel(Client *client, std::string &channel_name);
		void								unlinkClientFromChannel(Client *client, std::string &channel_name);
		
		// Pour PRIVMSG
		int									send_to_channel(Client &client, std::vector<std::string> &args);
		int									send_to_user(Client &client, std::vector<std::string> &args);
		
		int									executeCommand(Client &client, std::string &command, std::vector<int> &fdsToClose);
		
		void								do_cap(Client &client, std::vector<std::string> &args);
		int									checker_password(Client &client, std::vector<int> &fdsToClose, std::vector<std::string> &args);

		int									nickname(Client &client, std::vector<std::string> &args);			 	
		int 								user(Client &client, std::vector<std::string> &args);		

		int									do_join(Client &client, std::vector<std::string> &args);
		void								ping_pong(Client &client, std::vector<std::string> &args);
		void								do_msg(Client &client, std::vector<std::string> &args);
		void								do_mode(Client &client, std::vector<std::string> &args);
		int									do_part(Client &client, std::vector<std::string> &args);
		int									do_kick(Client &client, std::vector<std::string> &args);
		int									quit(Client &client, std::vector<int> &fdsToClose, std::vector<std::string> &args);

		int									do_invite(Client &client, std::vector<std::string> &args);
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
		
		int									findUser(std::string &username);
		int									parseNickname(std::string &nickname);
		
		int									init();
		int									run();

};

#endif
