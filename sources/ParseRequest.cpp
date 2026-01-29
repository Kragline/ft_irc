#include <ParseRequest.hpp>

ParseRequest::ParseRequest() {}

ParseRequest::ParseRequest(const std::string &line)
{
	std::size_t	position = 0, tokenStart = 0;

	while (tokenStart < line.size())
	{
		position = line.find("\r\n", tokenStart);
		if (position == std::string::npos)
			break ;
		_tokens.push_back(line.substr(tokenStart, position - tokenStart));
		tokenStart = position + 2;
	}
	// for (size_t i = 0; i < _tokens.size(); i++)
	// {
	// 	std::cout << _tokens[i] << std::endl;
	// }
}

ParseRequest::ParseRequest(const ParseRequest &other) : _tokens(other._tokens) {}
ParseRequest::~ParseRequest() {}

ParseRequest	&ParseRequest::operator=(const ParseRequest &other)
{
	if (&other != this)
		_tokens = other._tokens;
	return (*this);
}

const std::vector<std::string>	&ParseRequest::getTokens() const { return (_tokens); }
void	ParseRequest::resetParser() { _tokens.clear(); }
