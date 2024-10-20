#include "Server.hpp"
#include "Client.hpp"
#include "Replies.hpp"



void Server::authenticateClient(int clientFd, std::string command)
{
	Client *client = GetClient(clientFd);
	command = command.substr(4);
	size_t pos = command.find_first_not_of("\t\v ");
	if(pos < command.size())
	{
		command = command.substr(pos);
		if(command[0] == ':')
			command.erase(command.begin());
	}
	if(pos == std::string::npos || command.empty()) 
		_sendResponse(ERR_NOTENOUGHPARAM(std::string("*")), clientFd);
	else if(!client->getRegistered())
	{
		std::string passphrase = command;
		if(passphrase == password)
			client->setRegistered(true);
		else
            _sendResponse(ERR_INCORPASS(std::string("*")), clientFd);
	}
	else
        _sendResponse(ERR_ALREADYREGISTERED(GetClient(clientFd)->GetNickName()), clientFd);
}


bool Server::isNicknameValid(std::string& nickname)
{
	if(!nickname.empty() && (nickname[0] == '&' || nickname[0] == '#' || nickname[0] == ':'))
		return false;
	for(size_t i = 1; i < nickname.size(); i++)
	{
		if(!std::isalnum(nickname[i]) && nickname[i] != '_')
			return false;
	}
	return true;
}


bool Server::isNicknameInUse(std::string& nickname)
{
	for (size_t i = 0; i < this->clients.size(); i++)
	{
		if (this->clients[i].GetNickName() == nickname)
			return true;
	}
	return false;
}


void Server::assignNickname(int clientFd, std::string command)
{
	std::string inUseNickname;
	command = command.substr(4);
	size_t pos = command.find_first_not_of("\t\v ");
	if(pos < command.size())
	{
		command = command.substr(pos);
		if(command[0] == ':')
			command.erase(command.begin());
	}
	Client *client = GetClient(clientFd);
	if(pos == std::string::npos || command.empty())
		{_sendResponse(ERR_NOTENOUGHPARAM(std::string("*")), clientFd); return;}
	if (isNicknameInUse(command) && client->GetNickName() != command){
		
	    _sendResponse(ERR_NICKINUSE(std::string(command)), clientFd); 
		return;
	}
	if(!isNicknameValid(command)) {
		_sendResponse(ERR_ERRONEUSNICK(std::string(command)), clientFd);
		return;
	}
	else
	{
		if(client && client->getRegistered())
		{
			std::string previousNick = client->GetNickName();
			client->SetNickname(command);
			for(size_t i = 0; i < channels.size(); i++){
      	 	 Client *cl = channels[i].GetClientInChannel(previousNick);
       		 if(cl)
        	  cl->SetNickname(command);
      		}
			if(!previousNick.empty() && previousNick != command)
			{
				if(previousNick == "*" && !client->GetUserName().empty())
				{
					// client->setLogedin(true);
					_sendResponse(RPL_CONNECTED(client->GetNickName()), clientFd);
					_sendResponse(RPL_NICKCHANGE(client->GetNickName(), command), clientFd);
				}
				else
					_sendResponse(RPL_NICKCHANGE(previousNick, command), clientFd);
				return;
			}
			
		}
		else if (client && !client->getRegistered())
			_sendResponse(ERR_NOTREGISTERED(command), clientFd);
	}
	if(client && client->getRegistered() && !client->GetUserName().empty() && !client->GetNickName().empty() && client->GetNickName() != "*" ) //&& !client->GetIn()
	{
		// client->setLogedin(true);
		_sendResponse(RPL_CONNECTED(client->GetNickName()), clientFd);
	}
}

void	Server::assignUsername(std::string& command, int clientFd)
{
	std::vector<std::string> commandParts = splitInputCommand(command);

	Client *client = GetClient(clientFd); 
	if((client && commandParts.size() < 5))
		{_sendResponse(ERR_NOTENOUGHPARAM(client->GetNickName()), clientFd); return; }
	if(!client  || !client->getRegistered())
		_sendResponse(ERR_NOTREGISTERED(std::string("*")), clientFd);
	else if (client && !client->GetUserName().empty())
		{_sendResponse(ERR_ALREADYREGISTERED(client->GetNickName()), clientFd); return;}
	else
		client->SetUsername(commandParts[1]);
	if(client && client->getRegistered() && !client->GetUserName().empty() && !client->GetNickName().empty() && client->GetNickName() != "*"  ) //&& !client->GetLogedIn()
	{
		// client->setLogedin(true);
		_sendResponse(RPL_CONNECTED(client->GetNickName()), clientFd);
	}
}
