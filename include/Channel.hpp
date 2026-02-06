#pragma once

#include "Operator.hpp"

class Channel
{
private:
	const Operator				*_operator;
	std::vector<const Client *>	_members;

public:
	Channel(const Operator *op);
	Channel(const Channel &other);

	Channel	&operator=(const Channel &other);

	~Channel();

	const Operator				&getOpearator() const ;
	std::vector<const Client *>	&getMembers();
};
