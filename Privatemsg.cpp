#include "Server.hpp"

void ExtractCommand(std::string cmd, std::string to_find, std::string &message) {
    size_t index = 0;

    while (index < cmd.size()) {
        if (cmd[index] != ' ') {
            std::string token;
            while (index < cmd.size() && cmd[index] != ' ')
                token += cmd[index++];
            if (token == to_find) break;
        }
        ++index;
    }
    if (index < cmd.size())
        message = cmd.substr(index);

    index = 0;
    while (index < message.size() && message[index] == ' ')
        ++index;

    message = message.substr(index);
}

std::string ParseCommand(std::string &cmd, std::vector<std::string> &tokens) {
    std::stringstream stream(cmd);
    std::string word, message;
    int token_limit = 2;

    while (stream >> word && token_limit--)
        tokens.push_back(word);

    if (tokens.size() != 2)
        return std::string("");

    ExtractCommand(cmd, tokens[1], message);
    return message;
}

std::string ProcessPrivateMessage(std::string cmd, std::vector<std::string> &targets) {
    std::string message = ParseCommand(cmd, targets);

    if (targets.size() != 2) {
        targets.clear();
        return std::string("");
    }
    targets.erase(targets.begin());
    std::string recipients_str = targets[0];
    std::string recipient;

    targets.clear();
    for (size_t i = 0; i < recipients_str.size(); ++i) {
        if (recipients_str[i] == ',') {
            targets.push_back(recipient);
            recipient.clear();
        } else {
            recipient += recipients_str[i];
        }
    }
    targets.push_back(recipient);

    for (size_t i = 0; i < targets.size(); ++i) {
        if (targets[i].empty())
            targets.erase(targets.begin() + i--);
    }

    if (!message.empty() && message[0] == ':') {
        message.erase(message.begin());
    } else {
        for (size_t i = 0; i < message.size(); ++i) {
            if (message[i] == ' ') {
                message = message.substr(0, i);
                break;
            }
        }
    }

    return message;
}

void Server::ValidateRecipients(std::vector<std::string> &recipients, int fd) {
    for (size_t i = 0; i < recipients.size(); ++i) {
        if (recipients[i][0] == '#') {
            recipients[i].erase(recipients[i].begin());

            if (!GetChannel(recipients[i])) {
                senderror(401, "#" + recipients[i], GetClient(fd)->GetFd(), " :No such nick/channel\r\n");
                recipients.erase(recipients.begin() + i--);
            }
            else if (!GetChannel(recipients[i])->GetClientInChannel(GetClient(fd)->GetNickName())) {
                senderror(404, GetClient(fd)->GetNickName(), "#" + recipients[i], GetClient(fd)->GetFd(), " :Cannot send to channel\r\n");
                recipients.erase(recipients.begin() + i--);
            } else {
                recipients[i] = "#" + recipients[i];
            }
        } else {
            if (!GetClientNick(recipients[i])) {
                senderror(401, recipients[i], GetClient(fd)->GetFd(), " :No such nick/channel\r\n");
                recipients.erase(recipients.begin() + i--);
            }
        }
    }
}

void Server::PrivateMessage(std::string cmd, int fd) {
    std::vector<std::string> recipients;
    std::string message = ProcessPrivateMessage(cmd, recipients);

    if (recipients.empty()) {
        senderror(411, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), " :No recipient given (PRIVMSG)\r\n");
        return;
    }
    if (message.empty()) {
        senderror(412, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), " :No text to send\r\n");
        return;
    }
    if (recipients.size() > 10) {
        senderror(407, GetClient(fd)->GetNickName(), GetClient(fd)->GetFd(), " :Too many recipients\r\n");
        return;
    }

    ValidateRecipients(recipients, fd);

    for (size_t i = 0; i < recipients.size(); ++i) {
        std::string response;
        if (recipients[i][0] == '#') {
            std::string channel_name = recipients[i].substr(1);
            response = ":" + GetClient(fd)->GetNickName() + "!~" + GetClient(fd)->GetUserName() + "@localhost PRIVMSG #" + channel_name + " :" + message + "\r\n";
            GetChannel(channel_name)->sendTo_all(response, fd);
        } else {
            response = ":" + GetClient(fd)->GetNickName() + "!~" + GetClient(fd)->GetUserName() + "@localhost PRIVMSG " + recipients[i] + " :" + message + "\r\n";
            _sendResponse(response, GetClientNick(recipients[i])->GetFd());
        }
    }
}
