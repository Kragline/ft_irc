#include <ParseRequest.hpp>

ParseRequest::ParseRequest() {}

ParseRequest::ParseRequest(const std::string &line) { parseLine(line); }

ParseRequest::ParseRequest(const ParseRequest &other) : _tokens(other._tokens) {}
ParseRequest::~ParseRequest() {}

ParseRequest	&ParseRequest::operator=(const ParseRequest &other)
{
	if (&other != this)
		_tokens = other._tokens;
	return (*this);
}

void	ParseRequest::parseLine(const std::string &line)
{
	std::size_t	position = 0, tokenStart = 0;

	resetParser();
	while (tokenStart < line.size())
	{
		position = line.find("\r\n", tokenStart);
		if (position == std::string::npos)
			break ;
		_tokens.push_back(line.substr(tokenStart, position - tokenStart));
		tokenStart = position + 2;
	}
}

const std::vector<std::string>	&ParseRequest::getTokens() const { return (_tokens); }
void	ParseRequest::resetParser() { if (!_tokens.empty()) _tokens.clear(); }
