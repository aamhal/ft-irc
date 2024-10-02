#include "Server.hpp"

std::string Server::appendArg(std::string modeChain, char sign, char mode)
{
	std::stringstream ss;

	char last = '\0';
	for(size_t i = 0; i < modeChain.size(); i++)
	{
		if(modeChain[i] == '+' || modeChain[i] == '-')
			last = modeChain[i];
	}
	if(last != sign)
		ss << sign << mode;
	else
		ss << mode;
	return ss.str();
}

void Server::getCmdArgs(std::string cmd, std::string& name, std::string& modeSet, std::string& parameters)
{
	std::istringstream stm(cmd);
	stm >> name;
	stm >> modeSet;
	size_t found = cmd.find_first_not_of(name + modeSet + " \t\v");
	if(found != std::string::npos)
		parameters = cmd.substr(found);
}

std::vector<std::string> Server::splitParameters(std::string parameters)
{
	if(!parameters.empty() && parameters[0] == ':')
		parameters.erase(parameters.begin());
	std::vector<std::string> tokens;
	std::string param;
	std::istringstream stm(parameters);
	while (std::getline(stm, param, ','))
	{
		tokens.push_back(param);
		param.clear();
	}
	return tokens;
}

void Server::mode(std::string& cmd, int clientFd)
{
	std::string channelName;
	std::string parameters;
	std::string modeSet;
	std::stringstream modeChain;
	std::string arguments;
	Channel* channel;
	char sign;

	Client* client = GetClient(clientFd);
	size_t found = cmd.find_first_not_of("MODEmode \t\v");
	if(found != std::string::npos)
		cmd = cmd.substr(found);
	else
	{
		_sendResponse(ERR_NOTENOUGHPARAM(client->GetNickName()), clientFd); 
		return;
	}
	getCmdArgs(cmd, channelName, modeSet, parameters);
	std::vector<std::string> tokens = splitParameters(parameters);
	if(channelName[0] != '#' || !(channel = GetChannel(channelName.substr(1))))
	{
		_sendResponse(ERR_CHANNELNOTFOUND(client->GetUserName(), channelName), clientFd);
		return;
	}
	else if (!channel->get_client(clientFd) && !channel->get_admin(clientFd))
	{
		senderror(442, GetClient(clientFd)->GetNickName(), channelName, GetClient(clientFd)->GetFd(), " :You're not on that channel\r\n"); 
		return;
	}
	else if (modeSet.empty()) // response with the channel modes (MODE #channel)
	{
		_sendResponse(RPL_CHANNELMODES(client->GetNickName(), channel->GetName(), channel->getM()) + \
		RPL_CREATIONTIME(client->GetNickName(), channel->GetName(), channel->get_creationtime()), clientFd);
		return;
	}
	else if (!channel->get_admin(clientFd)) // client is not channel operator
	{
		_sendResponse(ERR_NOTOPERATOR(channel->GetName()), clientFd);
		return;
	}
	else if(channel)
	{
		size_t tokenIndex = 0;
		for(size_t i = 0; i < modeSet.size(); i++)
		{
			if(modeSet[i] == '+' || modeSet[i] == '-')
				sign = modeSet[i];
			else
			{
				if(modeSet[i] == 'i') // invite mode
					modeChain << invite_only(channel, sign, modeChain.str());
				else if (modeSet[i] == 't') // topic restriction mode
					modeChain << toggleTopicRestriction(channel, sign, modeChain.str());
				else if (modeSet[i] == 'k') // password set/remove
					modeChain << togglePasswordRestriction(tokens, channel, tokenIndex, sign, clientFd, modeChain.str(), arguments);
				else if (modeSet[i] == 'o') // set/remove user operator privilege
					modeChain << signe_privilege(tokens, channel, tokenIndex, clientFd, sign, modeChain.str(), arguments);
				else if (modeSet[i] == 'l') // set/remove channel limits
					modeChain << toggleChannelLimit(tokens, channel, tokenIndex, sign, clientFd, modeChain.str(), arguments);
				else
					_sendResponse(ERR_UNKNOWNMODE(client->GetNickName(), channel->GetName(), modeSet[i]), clientFd);
			}
		}
	}
	std::string modeResponseChain = modeChain.str();
	if(modeResponseChain.empty())
		return;
	channel->sendTo_all(RPL_CHANGEMODE(client->getHostname(), channel->GetName(), modeResponseChain, arguments));
}

std::string Server::invite_only(Channel* channel, char sign, std::string modeChain)
{
	std::string modeResponse;

	if(sign == '+' && !channel->getModeAtindex(0))
	{
		channel->setModeAtindex(0, true);
		channel->SetInvitOnly(1);
		modeResponse = appendArg(modeChain, sign, 'i');
	}
	else if (sign == '-' && channel->getModeAtindex(0))
	{
		channel->setModeAtindex(0, false);
		channel->SetInvitOnly(0);
		modeResponse = appendArg(modeChain, sign, 'i');
	}
	return modeResponse;
}

std::string Server::toggleTopicRestriction(Channel* channel, char sign, std::string modeChain)
{
	std::string modeResponse;
	modeResponse.clear();
	if(sign == '+' && !channel->getModeAtindex(1))
	{
		channel->setModeAtindex(1, true);
		channel->set_topicRestriction(true);
		modeResponse = appendArg(modeChain, sign, 't');
	}
	else if (sign == '-' && channel->getModeAtindex(1))
	{
		channel->setModeAtindex(1, false);
		channel->set_topicRestriction(false);
		modeResponse = appendArg(modeChain, sign, 't');
	}
	return modeResponse;
}

bool isPasswordValid(std::string password)
{
	if(password.empty())
		return false;
	for(size_t i = 0; i < password.size(); i++)
	{
		if(!std::isalnum(password[i]) && password[i] != '_')
			return false;
	}
	return true;
}

std::string Server::togglePasswordRestriction(std::vector<std::string> tokens, Channel* channel, size_t& tokenIndex, char sign, int clientFd, std::string modeChain, std::string& arguments)
{
	std::string modeResponse;
	std::string pass;

	if(tokens.size() > tokenIndex)
		pass = tokens[tokenIndex++];
	else
	{
		_sendResponse(ERR_NEEDMODEPARM(channel->GetName(), std::string("(k)")), clientFd);
		return modeResponse;
	}
	if(!isPasswordValid(pass))
	{
		_sendResponse(ERR_INVALIDMODEPARM(channel->GetName(), std::string("(k)")), clientFd);
		return modeResponse;
	}
	if(sign == '+')
	{
		channel->setModeAtindex(2, true);
		channel->SetPassword(pass);
		if(!arguments.empty())
			arguments += " ";
		arguments += pass;
		modeResponse = appendArg(modeChain, sign, 'k');
	}
	else if (sign == '-' && channel->getModeAtindex(2))
	{
		if(pass == channel->GetPassword())
		{
			channel->setModeAtindex(2, false);
			channel->SetPassword("");
			modeResponse = appendArg(modeChain, sign, 'k');
		}
		else
			_sendResponse(ERR_KEYSET(channel->GetName()), clientFd);
	}
	return modeResponse;
}

std::string Server::signe_privilege(std::vector<std::string> tokens, Channel* channel, size_t& tokenIndex, int clientFd, char sign, std::string modeChain, std::string& arguments)
{
	std::string user;
	std::string modeResponse;

	if(tokens.size() > tokenIndex)
		user = tokens[tokenIndex++];
	else
	{
		_sendResponse(ERR_NEEDMODEPARM(channel->GetName(), "(o)"), clientFd);
		return modeResponse;
	}
	if(!channel->clientInChannel(user))
	{
		_sendResponse(ERR_NOSUCHNICK(channel->GetName(), user), clientFd);
		return modeResponse;
	}
	if(sign == '+')
	{
		channel->setModeAtindex(3, true);
		if(channel->change_clientToAdmin(user))
		{
			modeResponse = appendArg(modeChain, sign, 'o');
			if(!arguments.empty())
				arguments += " ";
			arguments += user;
		}
	}
	else if (sign == '-')
	{
		channel->setModeAtindex(3, false);
		if(channel->change_adminToClient(user))
		{
			modeResponse = appendArg(modeChain, sign, 'o');
			if(!arguments.empty())
				arguments += " ";
			arguments += user;
		}
	}
	return modeResponse;
}

bool Server::isLimitValid(std::string& limit)
{
	return (!(limit.find_first_not_of("0123456789") != std::string::npos) && std::atoi(limit.c_str()) > 0);
}

std::string Server::toggleChannelLimit(std::vector<std::string> tokens, Channel* channel, size_t& tokenIndex, char sign, int clientFd, std::string modeChain, std::string& arguments)
{
	std::string limit;
	std::string modeResponse;

	if(tokens.size() > tokenIndex)
		limit = tokens[tokenIndex++];
	else
	{
		_sendResponse(ERR_NEEDMODEPARM(channel->GetName(), "(l)"), clientFd);
		return modeResponse;
	}
	if(!isLimitValid(limit))
	{
		_sendResponse(ERR_INVALIDMODEPARM(channel->GetName(), "(l)"), clientFd);
		return modeResponse;
	}
	if(sign == '+')
	{
		channel->setModeAtindex(4, true);
		channel->SetLimit(atoi(limit.c_str()));
		if(!arguments.empty())
			arguments += " ";
		arguments += limit;
		modeResponse = appendArg(modeChain, sign, 'l');
	}
	else if (sign == '-')
	{
		channel->setModeAtindex(4, false);
		channel->SetLimit(-1);
		modeResponse = appendArg(modeChain, sign, 'l');
	}
	return modeResponse;
}
