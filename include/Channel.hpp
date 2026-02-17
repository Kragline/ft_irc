#pragma once

#include "Operator.hpp"

class Channel
{
private:
	const Operator				*_operator;
	std::vector<const Client *>	_members;
    std::string                 _name;

public:
	Channel(const Operator *op);
	Channel(const Channel &other);

	Channel	&operator=(const Channel &other);

	~Channel();

	const Operator				&getOperator() const ;
	std::vector<const Client *>	&getMembers();

    std::string                 getName(void) const ;
    void                        setName(std::string name);
};
