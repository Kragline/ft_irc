#include "Channel.hpp"

Channel::Channel(const Operator *op) : _operator(op) {}

Channel::Channel(const Channel &other) : _operator(other._operator), _members(other._members) {}

Channel	&Channel::operator=(const Channel &other)
{
	if (this == &other)
		return (*this);
	
	_operator = other._operator;
	_members = other._members;
	
	return (*this);
}

Channel::~Channel() {}

const Operator				&Channel::getOpearator() const { return (*_operator); }
std::vector<const Client *>	&Channel::getMembers() { return (_members); }
