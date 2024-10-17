#include "Client.hpp"

Client::Client()
{
	this->nickname = "";
	this->username = "";
	this->fd = -1;
	this->isOperator= false;
	this->registered = false;
	this->buffer = "";
	this->ipadd = "";
	// this->logedin = false;
}
Client::Client(std::string nickname, std::string username, int fd) :fd(fd), nickname(nickname), username(username){}
Client::~Client(){}

//---------------//Getters
bool Client::GetInviteChannel(std::string &ChName){
	for (size_t i = 0; i < this->ChannelsInvite.size(); i++){
		if (this->ChannelsInvite[i] == ChName)
			return true;
	}
	return false;
}
int Client::GetFd(){return this->fd;}
bool Client::getRegistered(){return registered;}
std::string Client::GetNickName(){return this->nickname;}
std::string Client::GetUserName(){return this->username;}
std::string Client::getBuffer(){return buffer;}
std::string Client::getIpAdd(){return ipadd;}
std::string Client::getHostname(){
	std::string hostname = this->GetNickName() + "!" + this->GetUserName();
	return hostname;
}
//---------------//Getters
//---------------//Setters
void Client::SetFd(int fd){this->fd = fd;}
void Client::SetNickname(std::string& nickName){this->nickname = nickName;}
// void Client::setLogedin(bool value){this->in = value;}
void Client::SetUsername(std::string& username){this->username = username;}
void Client::setBuffer(std::string recived){buffer += recived;}
void Client::setRegistered(bool value){registered = value;}
void Client::setIpAdd(std::string ipadd){this->ipadd = ipadd;}
//---------------//Setters
//---------------//Methods
void Client::clearBuffer(){buffer.clear();}
// bool Client::GetLogedIn(){return this->logedin;}
void Client::AddChannelInvite(std::string &chname){
	ChannelsInvite.push_back(chname);
}
void Client::RmChannelInvite(std::string &chname){
	for (size_t i = 0; i < this->ChannelsInvite.size(); i++){
		if (this->ChannelsInvite[i] == chname)
			{this->ChannelsInvite.erase(this->ChannelsInvite.begin() + i); return;}
	}
}

