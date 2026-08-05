
#include "Channel.hpp"


// Constructeurs
Channel::Channel() : _name(""), _topic(""), _key(""), _userLimit(0), _inviteOnly(0), _topicOpOnly(0) {
	DEBUG(LOG_DEBUG << "[CHANNEL] : Creation.";);
}
Channel::Channel(const std::string &name) : _name(name),  _topic(""), _key(""), _userLimit(0), _inviteOnly(0), _topicOpOnly(0) {
	DEBUG(LOG_DEBUG << "[CHANNEL] : Creation with name `" << name << "`.";);

}

// Destructeur
Channel::~Channel() {
	DEBUG(LOG_DEBUG << "[CHANNEL] : Destruction.";);
}

// Getters
const std::string&				Channel::getName() const {
	return (_name);
}
const std::string&				Channel::getTopic() const {
	return (_topic);
}
const std::string&				Channel::getKey() const {
	return (_key);
}
const std::map<int, Client *>&	Channel::getMembers() const {
	return (_members);
}
uint8_t							Channel::getInviteOnly() const  {
	return _inviteOnly;
}
uint8_t							Channel::getTopicOpOnly() const {
	return _topicOpOnly;
}
size_t							Channel::getUserLimit() const   {
	return _userLimit;
}


Client* Channel::getMember(int fd) const {
	std::map<int, Client*>::const_iterator it = _members.find(fd);
	if (it != _members.end())
		return (it->second);
	return (NULL);
}

// Setters
void	Channel::setTopic(const std::string &topic) {
	_topic = topic;
}

void	Channel::setInviteOnly(uint8_t val)
{
	LOG_DEBUG << "setInviteOnly to " << static_cast<int>(val);
	_inviteOnly = val;
}

void	Channel::setTopicOpOnly(uint8_t val)
{
	LOG_DEBUG << "setTopicOpOnly to " << static_cast<int>(val);
	_topicOpOnly = val;
}

void	Channel::setUserLimit(size_t limit)
{
	LOG_DEBUG << "setUserLimit to " << limit;
	_userLimit = limit;
}

void	Channel::setKey(const std::string &key)
{
	LOG_DEBUG << "setKey to " << key;
	_key = key;
}

void	Channel::removeKey()
{
	LOG_DEBUG << "removeKey";
	_key.clear();
}



// Gestion des membres
void	Channel::addMember(Client *client) {
	if (!client)
		return ;
	//le 1e membre est celui qui cree le channel, il est donc operator
	if (_members.empty()) {
        addOperator(client->getFd());
    }
	_members.insert(std::make_pair(client->getFd(), client));
}

void	Channel::removeMember(int fd) {
	_members.erase(fd);
	_operators.erase(fd);
}

uint8_t	Channel::hasMember(int fd) const {
	return (getMember(fd) != NULL);
}


//Gestion des operateurs
void	Channel::addOperator(int fd) {
	_operators.insert(fd);
}
void	Channel::removeOperator(int fd) {
	_operators.erase(fd);
}
uint8_t	Channel::isOperator(int fd) const {
	return (_operators.count(fd) > 0);
}




















