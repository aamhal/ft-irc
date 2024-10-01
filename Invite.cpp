/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Invite.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akaabi <akaabi@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/27 10:44:11 by akaabi            #+#    #+#             */
/*   Updated: 2024/10/01 08:35:43 by akaabi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"


void Server::Invite(std::string &cmd, int &fd)
{
	std::vector<std::string> splited_command = split_cmd(cmd);
	if(splited_command.size() < 3)// bad param used
		{senderror(461, GetClient(fd)->GetNickName(), fd, " :Not enough parameters\r\n"); return;}
	std::string Name_channel = splited_command[2].substr(1);
	if(splited_command[2][0] != '#' || !GetChannel(Name_channel))//no such a channel name
	    {senderror(403, Name_channel, fd, " :No such channel\r\n"); return;}
	if (GetChannel(Name_channel)->GetClientInChannel(splited_command[1]))//the nickname trying to invite is already on that channel 
	    {senderror(443, GetClient(fd)->GetNickName(), Name_channel, fd, " :is already on channel\r\n"); return;}
	if (!(GetChannel(Name_channel)->get_client(fd)) && !(GetChannel(Name_channel)->get_admin(fd)))//client not in the that channel
	    {senderror(442, Name_channel, fd, " :You're not on that channel\r\n"); return;}
	Client *Client = GetClientNick(splited_command[1]);
	if (!Client)//no such a nickname found
		{senderror(401, splited_command[1], fd, " :No such nick\r\n");return;}
	if (GetChannel(Name_channel)->GetLimit() && GetChannel(Name_channel)->GetNumberofclient() >= GetChannel(Name_channel)->GetLimit()) //channe full 
		{senderror(473,GetChannel(Name_channel)->get_client(fd)->GetNickName(),Name_channel,fd," :Cannot invit to channel (+i)\r\n"); return;}
	if (GetChannel(Name_channel)->GetInvitOnly() && !GetChannel(Name_channel)->get_admin(fd))//cant invite if ur an admin
		{senderror(482,GetChannel(Name_channel)->get_client(fd)->GetNickName(),splited_command[1],fd," :You're not channel operator\r\n"); return;}
	//invite sent successfully
	Client->AddChannelInvite(Name_channel);
	std::string rep1 = ": 341 "+ GetClient(fd)->GetNickName()+" "+ Client->GetNickName()+" "+ splited_command[2]+"\r\n";
	_sendResponse(rep1, fd);
	std::string rep2 = ":"+ Client->getHostname() + " INVITE " + Client->GetNickName() + " " + splited_command[2]+"\r\n";
	_sendResponse(rep2, Client->GetFd());
	
}