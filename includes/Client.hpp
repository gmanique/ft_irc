#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "ft_irc.hpp"

class Client {
	private:
		int			_fd;
		std::string	_readBuffer;
	public:
		Client();
		Client(int fd);
		Client(int fd, std::string buf);
		Client(std::string buf);
		~Client();
		int					getFd() const;
		const std::string	&getBuffer() const;
		void				setFd(int fd);
		void				setBuffer(std::string buf);
		void				appendBuffer(std::string buf);
};


#endif
