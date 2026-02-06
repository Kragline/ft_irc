#pragma once

#include "Client.hpp"

class Operator : public Client
{
public:
	Operator(int fd);
	Operator(const Operator &other);

	Operator	&operator=(const Operator &other);

	~Operator();

	// TODO
	void	kick();
	void	invite();
	void	mode();
};
