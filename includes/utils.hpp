#ifndef UTILS_HPP
# define UTILS_HPP
# include "header.hpp"
# include "errno.h"
# include "signal.h"
class ClientConnection;
class Channel;
class Message;
class RPL;
class Client;
class Server;

std::vector<std::string> split( const std::string &  );
int	isNicknameAvailable( std::map<int, ClientConnection> &, std::string & );
int	verifNickname( std::string & );
std::string itoa(int );
bool isSpecial(char c);




#endif