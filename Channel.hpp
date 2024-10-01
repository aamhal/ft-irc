/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akaabi <akaabi@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/25 10:00:21 by akaabi            #+#    #+#             */
/*   Updated: 2024/10/01 08:47:51 by akaabi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"
#include "Server.hpp"



class Client;
class Channel {
   private:

	int only_invite;
	int topic;
	int key;
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
	void SetInvitOnly(int only_invite);
	void SetTopic(int topic);
	void SetKey(int key);
	void SetLimit(int limit);
	void SetTopicName(std::string topic_name);
	void SetPassword(std::string password);
	void SetName(std::string name);
	void SetTime(std::string time);
	void set_topicRestriction(bool value);
	void setModeAtindex(size_t index, bool mode);
	void set_createiontime();
	//---------------//Getters
	int GetInvitOnly();
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