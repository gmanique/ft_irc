#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "ft_irc.hpp"

# define PASSWORD_FLAG 	(1<<0)
# define NICKNAME_FLAG 	(1<<1)
# define USER_FLAG 		(1<<2)

# define HASPASSWORD(val) ((val & PASSWORD_FLAG) > 0)
# define SETHASPASSWORD(val) (val |= PASSWORD_FLAG)

# define HASNICKNAME(val) ((val & NICKNAME_FLAG) > 0)
# define SETHASNICKNAME(val) (val |= NICKNAME_FLAG)


# define HASUSER(val) ((val & USER_FLAG) > 0)
# define SETHASUSER(val) (val |= USER_FLAG)

// # define ISLOGGED(val) (val == 7)
# define ISLOGGED(val) (HASUSER(val) && HASNICKNAME(val) && HASPASSWORD(val))

class Client {
	private:
		int			_fd;
		std::string	_readBuffer;
		uint8_t		_isLogged;
		std::string _nickname;
		std::string _user;
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
		uint8_t				&getIsLogged() {return (_isLogged);}
		void				setNickname(std::string &nickname);
		const std::string 	&getNickname() const;
		void				setUser(std::string &nickname);
		const std::string 	&getUser() const;

		uint8_t				extractCommand(std::string &command);
};


#endif
