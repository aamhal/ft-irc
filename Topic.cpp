#include "Server.hpp"

std::string Server::TopicTime()
{
	std::time_t currentTime = std::time(NULL);
	std::stringstream timeStream;
	timeStream << currentTime;
	return timeStream.str();
}

std::string Server::extractTopic(const std::string &input)
{
	size_t delimiterPos = input.find(":");
	return (delimiterPos == std::string::npos) ? "" : input.substr(delimiterPos);
}

int Server::findColonPos(const std::string &command)
{
	for (size_t i = 1; i < command.size(); i++)
		if (command[i] == ':' && command[i - 1] == ' ')
			return i;
	return -1;
}

void Server::TopicCommand(const std::string &command, int &clientFd)
{
	if (command == "TOPIC :")
	{
		senderror(461, GetClient(clientFd)->GetNickName(), clientFd, " :Not enough parameters\r\n");
		return; // ERR_NEEDMOREPARAMS (461)
	}

	std::vector<std::string> commandParts = splitInputCommand(const_cast<std::string &>(command));
	if (commandParts.size() == 1)
	{
		senderror(461, GetClient(clientFd)->GetNickName(), clientFd, " :Not enough parameters\r\n");
		return; // ERR_NEEDMOREPARAMS (461)
	}

	std::string channelName = commandParts[1].substr(1);
	Channel *channel = GetChannel(channelName);

	if (!channel) // ERR_NOSUCHCHANNEL (403)
	{
		senderror(403, "#" + channelName, clientFd, " :No such channel\r\n");
		return;
	}

	bool isClientInChannel = channel->get_client(clientFd) || channel->get_admin(clientFd);

	if (!isClientInChannel) // ERR_NOTONCHANNEL (442)
	{
		senderror(442, "#" + channelName, clientFd, " :You're not on that channel\r\n");
		return;
	}

	// Handle existing topic
	if (commandParts.size() == 2)
	{
		const std::string &currentTopic = channel->GetTopicName();
		if (currentTopic.empty()) // RPL_NOTOPIC (331)
		{
			_sendResponse(": 331 " + GetClient(clientFd)->GetNickName() + " #" + channelName + " :No topic is set\r\n", clientFd);
		}
		else // RPL_TOPIC (332) and RPL_TOPICWHOTIME (333)
		{
			_sendResponse(": 332 " + GetClient(clientFd)->GetNickName() + " #" + channelName + " :" + currentTopic + "\r\n", clientFd);
			_sendResponse(": 333 " + GetClient(clientFd)->GetNickName() + " #" + channelName + " " + GetClient(clientFd)->GetNickName() + " " + channel->GetTime() + "\r\n", clientFd);
		}
		return;
	}

	// Handle topic setting
	std::vector<std::string> topicCommandParts;
	int colonPosition = findColonPos(command);
	if (colonPosition == -1 || commandParts[2][0] != ':')
	{
		topicCommandParts.push_back(commandParts[0]);
        topicCommandParts.push_back(commandParts[1]);
        topicCommandParts.push_back(commandParts[2]);

	}
	else
	{
		topicCommandParts.push_back(commandParts[0]);
        topicCommandParts.push_back(commandParts[1]);
        topicCommandParts.push_back(command.substr(colonPosition));
    
	}

	if (topicCommandParts[2] == ":") // RPL_NOTOPIC (331)
	{
		senderror(331, "#" + channelName, clientFd, " :No topic is set\r\n");
		return;
	}

	bool hasTopicRestrictions = channel->Gettopic_restriction();

	if (hasTopicRestrictions && !channel->get_admin(clientFd)) // ERR_CHANOPRIVSNEEDED (482)
	{
		senderror(482, "#" + channelName, clientFd, " :You're Not a channel operator\r\n");
		return;
	}

	// Update topic and notify the channel
	std::string newTopic = topicCommandParts[2];
	if (newTopic[0] == ':')
		newTopic = newTopic.substr(1);
	channel->SetTime(TopicTime());
	channel->SetTopicName(newTopic);

	std::string response = ":" + GetClient(clientFd)->GetNickName() + "!" + GetClient(clientFd)->GetUserName() + "@localhost TOPIC #" + channelName + " :" + newTopic + "\r\n";
	channel->sendTo_all(response);
}
