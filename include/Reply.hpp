#pragma once

#include "Client.hpp"

class Channel;

class Reply
{
private:
	Reply();
public:
	~Reply();

	static void	_noTopic(const Client &client, Channel *channel);
	static void	_topic(const Client &client, Channel *channel);
	static void	_inviting(const Client &client, const std::string &targetNick, const std::string &channelName);
	static void	_channelModeIs(const Client &client, const std::string &target, const std::string &currentModes);
	static void	_uModeIs(const Client &client, const std::string &modes);
};
