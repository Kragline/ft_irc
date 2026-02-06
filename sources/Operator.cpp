#include "Operator.hpp"

Operator::Operator(int fd) : Client(fd) {}

Operator::Operator(const Operator &other) : Client(other) {}

Operator	&Operator::operator=(const Operator &other)
{
	if (this == &other)
		return (*this);

	Client::operator=(other);
	return (*this);
}

Operator::~Operator() {}
