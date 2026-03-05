#include "Error.hpp"
#include "Client.hpp"

Error::Error() {}

Error::~Error() {}

void	Error::_alreadyRegistered(const Client &client)
{
	std::string msg = ":ircserv 462 " + (client.getNick().empty() ? "*" : client.getNick()) +
		" :You may not reregister\r\n";

	client.sendMessage(msg);
}

void	Error::_needMoreParams(const Client &client, const std::string &command)
{
	std::string nick = client.getNick().empty() ? "*" : client.getNick();

	std::string msg = ":ircserv 461 " + nick + " " + command +
						" :Not enough parameters\r\n";

	client.sendMessage(msg);
}

void	Error::_erroneousNickname(const Client &client, const std::string &nick)
{
	std::string msg = ":ircserv 432 " + nick + " :Erroneous nickname\r\n";

	client.sendMessage(msg);
}

void	Error::_nicknameInUse(const Client &client, const std::string &nick)
{
	std::string msg = ":ircserv 433 * " + nick + " :Nickname is already in use\r\n";

	client.sendMessage(msg);
}

void	Error::_passwordMismatch(const Client &client)
{
	client.sendMessage(":ircserv 464 * :Password incorrect\r\n");
}

void	Error::_noNicknameGiven(const Client &client)
{
	client.sendMessage(":ircserv 431 * :No nickname given\r\n");
}

void	Error::_notRegistered(const Client &client)
{
	client.sendMessage(":ircserv 451 * :You have not registered\r\n");
}

void	Error::_noSuchChannel(const Client &client, const std::string &channelName)
{
	std::string msg = ":ircserv 403 " + channelName + " :No such channel\r\n";

	client.sendMessage(msg);
}

void	Error::_chanOpPrivsNeeded(const Client &client, const std::string &channelName)
{
	std::string	msg = ":ircserv 482 " + client.getNick() + " " +
		channelName + " :You're not channel operator\r\n";

	client.sendMessage(msg);
}

void	Error::_notOnChannel(const Client &client, const std::string &channelName)
{
	std::string	msg = ":ircserv 442 " + channelName + " :You're not on that channel\r\n";

	client.sendMessage(msg);
}

void	Error::_noSuchNick(const Client &client, const std::string &nick)
{
	std::string msg = ":ircserv 401 " + nick + " :No such nick/channel\r\n";

	client.sendMessage(msg);
}

void	Error::_userOnChannel(const Client &client, const std::string &nick, const std::string &channelName)
{
	std::string msg = ":ircserv 443 " + nick + " " + channelName + " :is already on channel\r\n";

	client.sendMessage(msg);
}

void	Error::_inviteOnlyChan(const Client &client, const std::string &channelName)
{
	std::string msg = ":ircserv 473 " + client.getNick() + " " + channelName + " :Cannot join channel (+i)\r\n";

	client.sendMessage(msg);
}

void	Error::_badChanKey(const Client &client, const std::string &channelName)
{
	std::string msg = ":ircserv 475 " + client.getNick() + " " + channelName + " :Cannot join channel (+k)\r\n";

	client.sendMessage(msg);
}
