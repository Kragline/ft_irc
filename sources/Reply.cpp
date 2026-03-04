#include "Reply.hpp"

Reply::Reply() {}

Reply::~Reply() {}

void	Reply::_noTopic(const Client &client, Channel *channel)
{
	client.sendMessage(":ircserv 331 " + client.getNick() + " " + channel->getName() + " :No topic is set\r\n");
}

void	Reply::_topic(const Client &client, Channel *channel)
{
	client.sendMessage(":ircserv 332 " + client.getNick() + " " + channel->getName() + " :" + channel->getTopic() + "\r\n");
}

void	Reply::_inviting(const Client &client, const std::string &targetNick, const std::string &channelName)
{
	client.sendMessage(":ircserv 341 " + client.getNick() + " " + targetNick + " " + channelName + "\r\n");
}

void	Reply::_channelModeIs(const Client &client, const std::string &target, const std::string &currentModes)
{
	client.sendMessage(":ircserv 324 " + client.getNick() + " " + target + " " + currentModes + "\r\n");
}

void	Reply::_uModeIs(const Client &client, const std::string &modes)
{
	client.sendMessage(":ircserv 221 " + client.getNick() + " " + modes + "\r\n");
}

