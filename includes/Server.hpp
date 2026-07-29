#ifndef SERVER_HPP
# define SERVER_HPP

#include "ft_irc.hpp"

class Server {
	private:
		uint16_t	_port;	// normalement entre 6665 et 6669, le default est 6667
		std::string	_password;
	public:
		Server();
		Server(uint16_t	port);
		Server(uint16_t	port, std::string password);
		Server(std::string password);
		~Server();

		void		setPort(uint16_t port);
		void		setPassword(std::string password);
		uint16_t	getPort();
		std::string getPassword();
};

#endif
