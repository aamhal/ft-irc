
#pragma once

#include "Client.hpp"
#include "Server.hpp"



class Client;
class Channel {
   private:

	bool invite;
	bool key;
	int topic;
	int limit;
	bool restriction_T;
	std::string name;
	std::string time_creation;
	std::string password;
	std::string created_at;
	std::string topic_name;
	std::vector<Client> clients;
	std::vector<Client> admins;
	std::vector<std::pair<char, bool> > M;
public:
	Channel();
	~Channel();
	Channel(Channel const &src);
	Channel &operator=(Channel const &src);
	//---------------//Setters
	void setInvite(bool invite);
	void SetKey(int key);
	void SetTopic(int topic);
	void SetLimit(int limit);
	void SetTopicName(std::string topic_name);
	void SetPassword(std::string password);
	void SetName(std::string name);
	void SetTime(std::string time);
	void set_topicRestriction(bool value);
	void setModeAtindex(size_t index, bool mode);
	void set_createiontime();
	//---------------//Getters
	int getInvite();
	int GetTopic();
	int GetKey();
	int GetLimit();
	int GetNumberofclient();
	bool Getrestriction_T() const;
	bool getModeAtindex(size_t index);
	bool clientInChannel(std::string &nick);
	std::string GetTopicName();
	std::string GetPassword();
	std::string GetName();
	std::string GetTime();
	std::string get_creationtime();
	std::string getM();
	std::string clientChannel_list();
	Client *get_client(int fd);
	Client *get_admin(int fd);
	Client *GetClientInChannel(std::string name);
	bool Gettopic_restriction() const;

	//---------------//Methods
	void add_client(Client newClient);
	void add_admin(Client newClient);
	void remove_client(int fd);
	void remove_admin(int fd);
	bool change_clientToAdmin(std::string& nick);
	bool change_adminToClient(std::string& nick);
	//---------------//SendToAll
	void sendTo_all(std::string rpl1);
	void sendTo_all(std::string rpl1, int fd);
};