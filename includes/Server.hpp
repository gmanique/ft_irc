#ifndef SERVER_HPP
# define SERVER_HPP

#include "ft_irc.hpp"

class Client;

class Server {
	private:
		int						_port; // Port d'ecoute (entre 6665 et 6669)
		std::string				_password;
		int						_serverFd;
		std::vector<pollfd>		_pollFds;
		std::map<int, Client>	_clients; 

		/*
		void                	acceptNewClient();
		void                	handleClientData(int clientFd);
		void                	disconnectClient(int clientFd);
		*/

	public:
		Server();
		Server(int port, const std::string& password);
		~Server();

		void				setPort(int port);
		void				setPassword(const std::string& password);
		int					getPort() const;
		const std::string&	getPassword() const;

		int					init();
		int					run();

};

#endif
