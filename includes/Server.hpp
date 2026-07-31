#ifndef SERVER_HPP
# define SERVER_HPP

#include "ft_irc.hpp"

class Client;

class Server {
	private:
		int						_port;      // Port d'ecoute (entre 6665 et 6669)
		std::string				_password;  // Mot de passe du serveur
		int						_serverFd;  // FD du socket d'ecoute du serveur
		std::vector<pollfd>		_pollFds;   // Sockets surveilles
		std::map<int, Client>	_clients;   // Clients connectes (cle = fd)

	public:
		Server();
		Server(int port, const std::string& password);
		~Server();

		void				setPort(int port);
		void				setPassword(const std::string& password);
		int					getPort() const;
		const std::string&	getPassword() const;

		void				init();   // Cree le socket, bind, listen
		void				run();    // La boucle principale avec poll ou epoll ou autre

		/*
	private:
		void                acceptNewClient();
		void                handleClientData(int clientFd);
		void                disconnectClient(int clientFd);
*/
};

#endif
