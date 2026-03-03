#pragma once

#include "Client.hpp"

class Client;

class Error
{
private:
	Error();
	
public:
	static void	_alreadyRegistered(const Client &client);
	static void	_needMoreParams(const Client &client, const std::string &command);
	static void	_erroneousNickname(const Client &client, const std::string &nick);
	static void	_nicknameInUse(const Client &client, const std::string &nick);
	static void	_passwordMismatch(const Client &client);
	static void	_noNicknameGiven(const Client &client);
	static void	_notRegistered(const Client &client);
	static void	_noSuchChannel(const Client &client, const std::string &nameName);
	static void	_chanOpPrivsNeeded(const Client &client, const std::string &channelName);
	static void	_notOnChannel(const Client &client, const std::string &channelName);
	static void	_noSuchNick(const Client &client, const std::string &nick);
	static void	_userOnChannel(const Client &client, const std::string &nick, const std::string &channelName);
	static void	_inviteOnlyChan(const Client &client, const std::string &channelName);

	~Error();
};
