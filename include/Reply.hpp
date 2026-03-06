#pragma once

#include <string>

class Client;
class Channel;

inline void	NO_TOPIC(const Client &client, Channel *channel)
{
	client.sendMessage(":ircserv 331 " + client.getNick() + " " + channel->getName() + " :No topic is set\r\n");
}

inline void	TOPIC(const Client &client, Channel *channel)
{
	client.sendMessage(":ircserv 332 " + client.getNick() + " " + channel->getName() + " :" + channel->getTopic() + "\r\n");
}

inline void	INVITING(const Client &client, const std::string &targetNick, const std::string &channelName)
{
	client.sendMessage(":ircserv 341 " + client.getNick() + " " + targetNick + " " + channelName + "\r\n");
}

inline void	CHANNEL_MODE_IS(const Client &client, const std::string &target, const std::string &currentModes)
{
	client.sendMessage(":ircserv 324 " + client.getNick() + " " + target + " " + currentModes + "\r\n");
}

inline void	U_MODE_IS(const Client &client, const std::string &modes)
{
	client.sendMessage(":ircserv 221 " + client.getNick() + " " + modes + "\r\n");
}
