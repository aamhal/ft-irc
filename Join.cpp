#include "Server.hpp"


int Server::splitInputCommand_join(std::vector<std::pair<std::string, std::string> > &token, std::string cmd, int fd)
{
	std::vector<std::string> tmp;
	std::string ChStr, PassStr, buff;
	std::istringstream iss(cmd);
	while(iss >> cmd)
		tmp.push_back(cmd);
	if (tmp.size() < 2) {token.clear(); return 0;}
	tmp.erase(tmp.begin());
	ChStr = tmp[0]; tmp.erase(tmp.begin());
	if (!tmp.empty()) {PassStr = tmp[0]; tmp.clear();}
	for (size_t i = 0; i < ChStr.size(); i++){
		if (ChStr[i] == ',')
				{token.push_back(std::make_pair(buff, "")); buff.clear();}
		else buff += ChStr[i];
	}
	token.push_back(std::make_pair(buff, ""));
	if (!PassStr.empty()){
		size_t j = 0; buff.clear();
		for (size_t i = 0; i < PassStr.size(); i++){
			if (PassStr[i] == ',')
				{token[j].second = buff; j++; buff.clear();}
			else buff += PassStr[i];
		}
		token[j].second = buff;
	}
	for (size_t i = 0; i < token.size(); i++)//erase the empty channel names
		{if (token[i].first.empty())token.erase(token.begin() + i--);}
	for (size_t i = 0; i < token.size(); i++){//ERR_NOSUCHCHANNEL (403) // if the channel doesn't exist
		if (*(token[i].first.begin()) != '#')
			{senderror(403, GetClient(fd)->GetNickName(), token[i].first, GetClient(fd)->GetFd(), " :No such channel\r\n"); token.erase(token.begin() + i--);}
		else
			token[i].first.erase(token[i].first.begin());
	}
	return 1;
}

bool InvitedClient(Client *cli, std::string ChName, int flag){
	if(cli->GetInviteChannel(ChName)){
		if (flag == 1)
			cli->RmChannelInvite(ChName);
		return true;
	}
	return false;
}
int Server::Client_Finder(std::string nickname)
{
	int count = 0;
	for (size_t i = 0; i < this->channels.size(); i++){
		if (this->channels[i].GetClientInChannel(nickname))
			count++;
	}
	return count;
}


void Server::Found_Chennel(std::vector<std::pair<std::string, std::string> >&token, int i, int j, int fd)
{
	if (this->channels[j].GetLimit() && this->channels[j].GetNumberofclient() >= this->channels[j].GetLimit())// Channel is full of clients
		{senderror(471, GetClient(fd)->GetNickName(), "#" + token[i].first, GetClient(fd)->GetFd(), " :Cannot join channel (+l)\r\n"); return;}
	if (Client_Finder(GetClient(fd)->GetNickName()) >= 10)//already Reach 10 clients
		{senderror(405, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), " :You have joined too many channels\r\n"); return;}
	if (this->channels[j].getInvite()){//channel is INvite ONLY
		if (!InvitedClient(GetClient(fd), token[i].first, 1))
			{senderror(473, GetClient(fd)->GetNickName(), "#" + token[i].first, GetClient(fd)->GetFd(), " :Cannot join channel (+i)\r\n"); return;}
	}
	if (!this->channels[j].GetPassword().empty() && this->channels[j].GetPassword() != token[i].second){// Password incorrect
		if (!InvitedClient(GetClient(fd), token[i].first, 0))
			{senderror(475, GetClient(fd)->GetNickName(), "#" + token[i].first, GetClient(fd)->GetFd(), " :Cannot join channel (+k) - bad key\r\n"); return;}
	}
	if (this->channels[j].GetClientInChannel(GetClient(fd)->GetNickName()))// Client is registered already
		return;
	// adding the client
	Client *client = GetClient(fd);
	this->channels[j].add_client(*client);
	if(channels[j].GetTopicName().empty())
		_sendResponse(RPL_JOINMSG(GetClient(fd)->getHostname(),GetClient(fd)->getIpAdd(),token[i].first) + \
			RPL_NAMREPLY(GetClient(fd)->GetNickName(),channels[j].GetName(),channels[j].clientChannel_list()) + \
			RPL_ENDOFNAMES(GetClient(fd)->GetNickName(),channels[j].GetName()),fd);
	else
		_sendResponse(RPL_JOINMSG(GetClient(fd)->getHostname(),GetClient(fd)->getIpAdd(),token[i].first) + \
			RPL_TOPICIS(GetClient(fd)->GetNickName(),channels[j].GetName(),channels[j].GetTopicName()) + \
			RPL_NAMREPLY(GetClient(fd)->GetNickName(),channels[j].GetName(),channels[j].clientChannel_list()) + \
			RPL_ENDOFNAMES(GetClient(fd)->GetNickName(),channels[j].GetName()),fd);
    channels[j].sendTo_all(RPL_JOINMSG(GetClient(fd)->getHostname(),GetClient(fd)->getIpAdd(),token[i].first), fd);
}


void Server::NotFound_Chennel(std::vector<std::pair<std::string, std::string> >&token, int i, int fd)
{
	if (Client_Finder(GetClient(fd)->GetNickName()) >= 10)//U reached the limit of the channels can join
		{senderror(405, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), " :You have joined too many channels\r\n"); return;}
	Channel newChannel;
	newChannel.SetName(token[i].first);
	newChannel.add_admin(*GetClient(fd));
	newChannel.set_createiontime();
	this->channels.push_back(newChannel);
	//tell others that the client has join
    _sendResponse(RPL_JOINMSG(GetClient(fd)->getHostname(),GetClient(fd)->getIpAdd(),newChannel.GetName()) + \
        RPL_NAMREPLY(GetClient(fd)->GetNickName(),newChannel.GetName(),newChannel.clientChannel_list()) + \
        RPL_ENDOFNAMES(GetClient(fd)->GetNickName(),newChannel.GetName()),fd);
}

void Server::JOIN(std::string cmd, int fd)
{
	std::vector<std::pair<std::string, std::string> > token;
	// splitInputCommand_join(token, cmd, fd);
	if (token.size() > 10) //more than 10 Channels
		{senderror(407, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), " :Too many channels\r\n"); return;}
	if (!splitInputCommand_join(token, cmd, fd))//no channel name
		{senderror(461, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), " :Not enough parameters\r\n"); return;}
	for (size_t i = 0; i < token.size(); i++){
		bool flag = false;
		for (size_t j = 0; j < this->channels.size(); j++){
			if (this->channels[j].GetName() == token[i].first){
				Found_Chennel(token, i, j, fd);
				flag = true; break;
			}
		}
		if (!flag)
			NotFound_Chennel(token, i, fd);
	}
}