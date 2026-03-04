#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <limits>
#include <cstddef>

class Client;

class Channel
{
private:
	std::string				_name;
	std::string				_key;
	std::string				_topic;

	std::vector<Client *>	_operators;
	std::vector<Client *>	_members;
	std::vector<Client *>	_invited;

	bool					_inviteOnly;
	bool					_topicRestricted;
	bool					_hasKey;
	bool					_hasLimit;

	size_t					_limit;

public:
	Channel(const std::string &name, Client *op);
	Channel(const Channel &other);

	Channel	&operator=(const Channel &other);

	~Channel();

	std::vector<Client *>	&getOperators();
	std::vector<Client *>	&getMembers();
	std::vector<Client *>	&getInvited();

	std::string	getName() const ;
	void		setName(const std::string &name);

	bool		isInviteOnly() const;
	void		setInviteOnly(bool status);

	bool		isTopicRestricted() const ;
	void		setTopicRestricted(bool status);

	void		setKey(const std::string &newKey);
	void		removeKey();
	bool		hasKey() const ;
	std::string	getKey() const ;

	void		setTopic(const std::string &newTopic);
	std::string	getTopic() const ;

	void	setLimit(size_t newLimit);
	void	removeLimit();
	bool	hasLimit() const ;
	size_t	getLimit() const ;

	void	addMember(Client *client);
	void	addRandomOperator();
	void	removeMember(Client *client);

	bool	isMember(Client *client);
	bool	isEmpty() const ;

	void	addInvited(Client *client);
	bool	isInvited(Client *client);
	void	removeInvited(Client *client);

	void	addOperator(Client *client);
	bool	isOperator(Client *client);
	void	removeOperator(Client *client);

	size_t	operatorCount() const ;
	size_t	memberCount() const ;

	void	broadcast(const std::string &message, Client *exclude = NULL);

	std::string	getModeString() const ;

private:
    std::vector<Client *>::iterator	_findMember(std::vector<Client *> &vec, Client *client);
};
