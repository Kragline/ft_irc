#pragma once

#include <string>

#include "Client.hpp"

class Client;

inline void	ALREADY_REGISTERED(const Client &client)
{
	std::string msg = ":ircserv 462 " + (client.getNick().empty() ? "*" : client.getNick()) +
		" :You may not reregister\r\n";

	client.sendMessage(msg);
}

inline void	NEED_MORE_PARAMS(const Client &client, const std::string &command)
{
	std::string nick = client.getNick().empty() ? "*" : client.getNick();

	std::string msg = ":ircserv 461 " + nick + " " + command +
						" :Not enough parameters\r\n";

	client.sendMessage(msg);
}

inline void	ERRONEUS_NICKNAME(const Client &client, const std::string &nick)
{
	std::string msg = ":ircserv 432 " + nick + " :Erroneous nickname\r\n";

	client.sendMessage(msg);
}

inline void	NICKNAME_IN_USE(const Client &client, const std::string &nick)
{
	std::string msg = ":ircserv 433 * " + nick + " :Nickname is already in use\r\n";

	client.sendMessage(msg);
}

inline void	PASSWORD_MISMATCH(const Client &client)
{
	client.sendMessage(":ircserv 464 * :Password incorrect\r\n");
}

inline void	NO_NICKNAME_GIVEN(const Client &client)
{
	client.sendMessage(":ircserv 431 * :No nickname given\r\n");
}

inline void	NOT_REGISTERED(const Client &client)
{
	client.sendMessage(":ircserv 451 * :You have not registered\r\n");
}

inline void	NO_SUCH_CHANNEL(const Client &client, const std::string &channelName)
{
	std::string msg = ":ircserv 403 " + channelName + " :No such channel\r\n";

	client.sendMessage(msg);
}

inline void	CHAN_OP_PRIVS_NEEDED(const Client &client, const std::string &channelName)
{
	std::string	msg = ":ircserv 482 " + client.getNick() + " " +
		channelName + " :You're not channel operator\r\n";

	client.sendMessage(msg);
}

inline void	NOT_ON_CHANNEL(const Client &client, const std::string &channelName)
{
	std::string	msg = ":ircserv 442 " + channelName + " :You're not on that channel\r\n";

	client.sendMessage(msg);
}

inline void	NO_SUCH_NICK(const Client &client, const std::string &nick)
{
	std::string msg = ":ircserv 401 " + nick + " :No such nick/channel\r\n";

	client.sendMessage(msg);
}

inline void	USER_ON_CHANNEL(const Client &client, const std::string &nick, const std::string &channelName)
{
	std::string msg = ":ircserv 443 " + nick + " " + channelName + " :is already on channel\r\n";

	client.sendMessage(msg);
}

inline void	INVITE_ONLY_CHAN(const Client &client, const std::string &channelName)
{
	std::string msg = ":ircserv 473 " + client.getNick() + " " + channelName + " :Cannot join channel (+i)\r\n";

	client.sendMessage(msg);
}

inline void	BAD_CHAN_KEY(const Client &client, const std::string &channelName)
{
	std::string msg = ":ircserv 475 " + client.getNick() + " " + channelName + " :Cannot join channel (+k)\r\n";

	client.sendMessage(msg);
}

