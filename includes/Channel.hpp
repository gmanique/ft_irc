#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include "ft_irc.hpp"

class Client;

class Channel {
	private:
		std::string				_name;
		std::string				_topic;
		std::string				_key;         // Pour le mode +k
		size_t					_userLimit;   // Pour le mode +l (0 = pas de limite)

		// Modes : i, t, k, l
		uint8_t					_inviteOnly;  // mode +i
		uint8_t					_topicOpOnly; // mode +t

		std::map<int, Client*>	_members;     // fd -> Client*
		std::set<int>			_operators;   // fds des opérateurs du channel
		std::set<int>			_invitedFds;  // fds invités si mode +i

	public:
		Channel();
		Channel(const std::string &name);
		~Channel();

		//setters
		const std::string&				getName() const;
		const std::string&				getTopic() const;
		const std::string&				getKey() const;
		const std::map<int, Client*>&	getMembers() const;
		Client*							getMember(int fd) const;
		uint8_t							getInviteOnly() const;
		uint8_t							getTopicOpOnly() const;
		size_t							getUserLimit() const;
		
		//setters
		void	setTopic(const std::string &topic);
		void	setInviteOnly(uint8_t val);
		void	setTopicOpOnly(uint8_t val);
		void	setUserLimit(size_t limit);
		void	setKey(const std::string &key);
		void	removeKey();
	
		void	addInvMember(int fd);
		void	removeInvMember(int fd);
		uint8_t	hasInvMember(int fd) const;
		
		void	addMember(Client *client);
		void	removeMember(int fd);
		uint8_t	hasMember(int fd) const;
		
		void	addOperator(int fd);
		void	removeOperator(int fd);
		uint8_t	isOperator(int fd) const;

};

#endif
