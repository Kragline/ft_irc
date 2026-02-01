#pragma once

#include <iostream>
#include <vector>

class ParseRequest
{
private:
	std::vector<std::string>	_tokens;
public:
	ParseRequest();
	ParseRequest(const std::string &line);
	ParseRequest(const ParseRequest &other);
	~ParseRequest();

	ParseRequest	&operator=(const ParseRequest &other);

	const std::vector<std::string>	&getTokens() const ;
	void	parseLine(const std::string &line);
	void	resetParser();
};
