/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akaabi <akaabi@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/25 10:00:29 by akaabi            #+#    #+#             */
/*   Updated: 2024/10/01 08:47:04 by akaabi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(){
	this->only_invite = 0;
	this->topic = 0;
	this->key = 0;
	this->limit = 0;
	this->restriction_T = false;
	this->name = "";
	this->topic_name = "";
	char charaters[5] = {'i', 't', 'k', 'o', 'l'};
	for(int i = 0; i < 5; i++)
		M.push_back(std::make_pair(charaters[i],false));
	this->created_at = "";
}
Channel::~Channel(){}
Channel::Channel(Channel const &src){*this = src;}
Channel &Channel::operator=(Channel const &src){
	if (this != &src){
		this->only_invite = src.only_invite;
		this->topic = src.topic;
		this->key = src.key;
		this->limit = src.limit;
		this->restriction_T = src.restriction_T;
		this->name = src.name;
		this->password = src.password;
		this->created_at = src.created_at;
		this->topic_name = src.topic_name;
		this->clients = src.clients;
		this->admins = src.admins;
		this->M = src.M;
	}
	return *this;
}
//---------------//Setters
void Channel::SetInvitOnly(int only_invite){this->only_invite = only_invite;}
void Channel::SetTopic(int topic){this->topic = topic;}
void Channel::SetTime(std::string time){this->time_creation = time;}
void Channel::SetKey(int key){this->key = key;}
void Channel::SetLimit(int limit){this->limit = limit;}
void Channel::SetTopicName(std::string topic_name){this->topic_name = topic_name;}
void Channel::SetPassword(std::string password){this->password = password;}
void Channel::SetName(std::string name){this->name = name;}
void Channel::set_topicRestriction(bool value){this->restriction_T = value;}
void Channel::setModeAtindex(size_t index, bool mode){M[index].second = mode;}
void Channel::set_createiontime(){
	std::time_t _time = std::time(NULL);
	std::ostringstream oss;
	oss << _time;
	this->created_at = std::string(oss.str());
}
//---------------//Setters
//---------------//Getters
int Channel::GetInvitOnly(){return this->only_invite;}
int Channel::GetTopic(){return this->topic;}
int Channel::GetKey(){return this->key;}
int Channel::GetLimit(){return this->limit;}
int Channel::GetNumberofclient(){return this->clients.size() + this->admins.size();}
bool Channel::Getrestriction_T() const{return this->restriction_T;}
bool Channel::getModeAtindex(size_t index){return M[index].second;}
bool Channel::clientInChannel(std::string &nick){
	for(size_t i = 0; i < clients.size(); i++){
		if(clients[i].GetNickName() == nick)
			return true;
	}
	for(size_t i = 0; i < admins.size(); i++){
		if(admins[i].GetNickName() == nick)
			return true;
	}
	return false;
}
std::string Channel::GetTopicName(){return this->topic_name;}
std::string Channel::GetPassword(){return this->password;}
std::string Channel::GetName(){return this->name;}
std::string Channel::GetTime(){return this->time_creation;}
std::string Channel::get_creationtime(){return created_at;}
std::string Channel::getM(){
	std::string mode;
	for(size_t i = 0; i < M.size(); i++){
		if(M[i].first != 'o' && M[i].second)
			mode.push_back(M[i].first);
	}
	if(!mode.empty())
		mode.insert(mode.begin(),'+');
	return mode;
}
std::string Channel::clientChannel_list(){
	std::string list;
	for(size_t i = 0; i < admins.size(); i++){
		list += "@" + admins[i].GetNickName();
		if((i + 1) < admins.size())
			list += " ";
	}
	if(clients.size())
		list += " ";
	for(size_t i = 0; i < clients.size(); i++){
		list += clients[i].GetNickName();
		if((i + 1) < clients.size())
			list += " ";
	}
	return list;
}
Client *Channel::get_client(int fd){
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it){
		if (it->GetFd() == fd)
			return &(*it);
	}
	return NULL;
}
Client *Channel::get_admin(int fd){
	for (std::vector<Client>::iterator it = admins.begin(); it != admins.end(); ++it){
		if (it->GetFd() == fd)
			return &(*it);
	}
	return NULL;
}
Client* Channel::GetClientInChannel(std::string name)
{
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it){
		if (it->GetNickName() == name)
			return &(*it);
	}
	for (std::vector<Client>::iterator it = admins.begin(); it != admins.end(); ++it){
		if (it->GetNickName() == name)
			return &(*it);
	}
	return NULL;
}
//---------------//Getters
//---------------//Methods
void Channel::add_client(Client newClient){clients.push_back(newClient);}
void Channel::add_admin(Client newClient){admins.push_back(newClient);}
void Channel::remove_client(int fd){
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it){
		if (it->GetFd() == fd)
			{clients.erase(it); break;}
	}
}
void Channel::remove_admin(int fd){
	for (std::vector<Client>::iterator it = admins.begin(); it != admins.end(); ++it){
		if (it->GetFd() == fd)
			{admins.erase(it); break;}
	}
}
bool Channel::change_clientToAdmin(std::string& nick){
	size_t i = 0;
	for(; i < clients.size(); i++){
		if(clients[i].GetNickName() == nick)
			break;
	}
	if(i < clients.size()){
		admins.push_back(clients[i]);
		clients.erase(i + clients.begin());
		return true;
	}
	return false;
}

bool Channel::change_adminToClient(std::string& nick){
	size_t i = 0;
	for(; i < admins.size(); i++){
		if(admins[i].GetNickName() == nick)
			break;
	}
	if(i < admins.size()){
		clients.push_back(admins[i]);
		admins.erase(i + admins.begin());
		return true;
	}
	return false;

}
//---------------//Methods
//---------------//SendToAll
void Channel::sendTo_all(std::string rpl1)
{
	for(size_t i = 0; i < admins.size(); i++)
		if(send(admins[i].GetFd(), rpl1.c_str(), rpl1.size(),0) == -1)
			std::cerr << "send() faild" << std::endl;
	for(size_t i = 0; i < clients.size(); i++)
		if(send(clients[i].GetFd(), rpl1.c_str(), rpl1.size(),0) == -1)
			std::cerr << "send() faild" << std::endl;
}
void Channel::sendTo_all(std::string rpl1, int fd)
{
	for(size_t i = 0; i < admins.size(); i++){
		if(admins[i].GetFd() != fd)
			if(send(admins[i].GetFd(), rpl1.c_str(), rpl1.size(),0) == -1)
				std::cerr << "send() faild" << std::endl;
	}
	for(size_t i = 0; i < clients.size(); i++){
		if(clients[i].GetFd() != fd)
			if(send(clients[i].GetFd(), rpl1.c_str(), rpl1.size(),0) == -1)
				std::cerr << "send() faild" << std::endl;
	}
}
//---------------//SendToAll