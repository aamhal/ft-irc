
#include "Server.hpp"

void Find_reason(std::string command, std::string tofind, std::string &str)
{
	size_t i = 0;
	for (; i < command.size(); i++){
		if (command[i] != ' '){
			std::string temp;
			for (; i < command.size() && command[i] != ' '; i++)
				temp += command[i];
			if (temp == tofind)
                break;
			else
                temp.clear();
		}
	}
	if (i < command.size()) str = command.substr(i);
	i = 0;
	for (; i < str.size() && str[i] == ' '; i++);
	str = str.substr(i);
}

std::string Split_CMDFR(std::string &command, std::vector<std::string> &temp)
{
	std::string s, r;
	std::stringstream stringst(command);
	int count = 3;
	while (stringst >> s && count--)
		temp.push_back(s);
	if(temp.size() != 3)
        return std::string("");
	Find_reason(command, temp[2], r);
	return r;
}

std::string Server::Spilit_CMD_Kick(std::string command, std::vector<std::string> &temp, std::string &user, int fd)
{
	std::string reason = Split_CMDFR(command, temp);
	if (temp.size() < 3) // check if the param is correct
		return std::string("");
	temp.erase(temp.begin());
	std::string s = temp[0];
    std::string s1;
	user = temp[1]; temp.clear();
	for (size_t i = 0; i < s.size(); i++){//split cmd by ","
		if (s[i] == ',')
			{temp.push_back(s1); s1.clear();}
		else
            s1 += s[i];
	}
	temp.push_back(s1);
	for (size_t i = 0; i < temp.size(); i++)//delete data 
		{if (temp[i].empty())temp.erase(temp.begin() + i--);}
	if (reason[0] == ':') reason.erase(reason.begin());
	else 
		{for (size_t i = 0; i < reason.size(); i++){if (reason[i] == ' '){reason = reason.substr(0, i);break;}}}
	for (size_t i = 0; i < temp.size(); i++){// check channel is valid and delete #
			if (*(temp[i].begin()) == '#')
				temp[i].erase(temp[i].begin());
			else
				{senderror(403, GetClient(fd)->GetNickName(), temp[i], GetClient(fd)->GetFd(), " :No such channel\r\n"); temp.erase(temp.begin() + i--);}
		}
	return reason;
}

void	Server::KICK(std::string cmd, int fd)
{

	std::vector<std::string> tmp;
	std::string reason ,user;
	reason = Spilit_CMD_Kick(cmd, tmp, user, fd);
	if (user.empty())
		{senderror(461, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), " :Not enough parameters\r\n"); return;}
	for (size_t i = 0; i < tmp.size(); i++){ // searching for the channel
		if (GetChannel(tmp[i])){// check if the channel exist
			Channel *channel = GetChannel(tmp[i]);
			if (!channel->get_client(fd) && !channel->get_admin(fd)) // checking if the client already on that channel
				{senderror(442, GetClient(fd)->GetNickName(), "#" + tmp[i], GetClient(fd)->GetFd(), " :You're not on that channel\r\n"); continue;}
			if(channel->get_admin(fd)){ // checking if the client is an admin
				if (channel->GetClientInChannel(user)){ // checking if the client in on that channelto be kicked
					std::stringstream StringSt;
					StringSt << ":" << GetClient(fd)->GetNickName() << "!~" << GetClient(fd)->GetUserName() << "@" << "localhost" << " KICK #" << tmp[i] << " " << user;
					if (!reason.empty())
						StringSt << " :" << reason << "\r\n";
					else
                        StringSt << "\r\n";
					channel->sendTo_all(StringSt.str());
					if (channel->get_admin(channel->GetClientInChannel(user)->GetFd()))
						channel->remove_admin(channel->GetClientInChannel(user)->GetFd());
					else
						channel->remove_client(channel->GetClientInChannel(user)->GetFd());
					if (channel->GetNumberofclient() == 0)
						channels.erase(channels.begin() + i);
				}
				else // client not is that channel to be kicked
					{senderror(441, GetClient(fd)->GetNickName(), "#" + tmp[i], GetClient(fd)->GetFd(), " :They aren't on that channel\r\n"); continue;}
			}
			else // client is not an admin
				{senderror(482, GetClient(fd)->GetNickName(), "#" + tmp[i], GetClient(fd)->GetFd(), " :You're not channel operator\r\n"); continue;}
		}
		else // channel not exist
			senderror(403, GetClient(fd)->GetNickName(), "#" + tmp[i], GetClient(fd)->GetFd(), " :No such channel\r\n");
	}
}