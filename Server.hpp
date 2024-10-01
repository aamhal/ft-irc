#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <csignal>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <ctime>
#include "Client.hpp"
#include "Replies.hpp"
#include "Channel.hpp"


#define RED "\e[1;31m"
#define WHI "\e[0;37m"
#define GRE "\e[1;32m"
#define YEL "\e[1;33m"

class Client;
class Channel;

class Server
{
private:
	int server_fdsocket;
	std::vector<Channel> channels;
	struct sockaddr_in cliadd;
	struct sockaddr_in add;
	struct pollfd new_cli;
	std::vector<struct pollfd> fds;
	std::vector<Client> clients;
	int my_port;
	std::string password;
	static bool Signal;
public:
	Server();
	~Server();
	//---------------//Getters
	Client *GetClientNick(std::string nickname);
	int GetFd();
	int GetPort();
	std::string GetPassword();
	Client *GetClient(int fd);
	Channel *GetChannel(std::string name);      
	static bool isBotfull;
	//---------------//Setters
	void SetPort(int port);
	void set_nickname(int fd, std::string cmd);
	void set_username(std::string& username, int fd);
	void AddChannel(Channel newChannel);
	void AddClient(Client newClient);
	void AddFds(pollfd newFd);
	void SetFd(int server_fdsocket);
	void SetPassword(std::string password);
	//---------------//Remove Methods
	void RemoveFds(int fd);
	void RemoveClient(int fd);
	void RemoveChannel(std::string name);
	//---------------//Send Methods
	void senderror(int code, std::string clientname, int fd, std::string msg);
	void senderror(int code, std::string clientname, std::string channelname, int fd, std::string msg);
	void 		_sendResponse(std::string response, int fd);
	//---------------//Close and Signal Methods
	static void SignalHandler(int signum);
	void close_fds();
	//---------------//Server Methods
	void init(int port, std::string pass);
	void set_sever_socket();
	void reciveNewData(int fd);
	void accept_new_client();
	//---------------//Parsing Methods
	void parse_exec_cmd(std::string &cmd, int fd);
	std::vector<std::string> split_cmd(std::string &str);
	std::vector<std::string> split_recivedBuffer(std::string str);
	//---------------//Authentification Methods
	void client_authen(int fd, std::string pass);
	bool is_validNickname(std::string& nickname);
	bool nickNameInUse(std::string& nickname);
	bool BypassForBot( int fd, std::string cmd);
	bool notregistered(int fd);
	//----------------// JOIN 
	int		Split_cmd_join(std::vector<std::pair<std::string, std::string> > &token, std::string cmd, int fd);
	int		Client_Finder(std::string nickname);
	void	Found_Chennel(std::vector<std::pair<std::string, std::string> >&token, int i, int j, int fd);
	void	NotFound_Chennel(std::vector<std::pair<std::string, std::string> >&token, int i, int fd);
	void	JOIN(std::string cmd, int fd);
	// --------------// Invite
	void Invite(std::string &cmd, int &fd);
};
