#ifndef RPL_HPP
# define RPL_HPP
# include "header.hpp"
# include "ClientConnection.hpp"
# include "Client.hpp"
# include "Channel.hpp"
# include "Server.hpp"
# include "Message.hpp"

// /connect DALNET + /RAWLOG OPEN debug.log for debug logs on official server (to test display)
class ClientConnection;
class Server;

class RPL
{
    private:
		std::string		name;
		std::string		topic;
        std::set<int>	users;
        std::set<int>	operators;

    public:
        RPL( void );
        ~RPL( void );

		static std::string	rplWelcome( const std::string & );
		static std::string	rplNoTopic( const std::string &, const std::string & );
		static std::string	rplTopic( const std::string &, const std::string &,  const std::string & );

		static std::string	rplInviting( const std::string &, const std::string &,  const std::string & );

		static std::string	errNeedMoreParams( const std::string & );
		static std::string	errNotOnChannel( const std::string & );
		static std::string	errChanOpPrivsNeeded( const std::string &, const std::string &);
		static std::string	errNoChanModes( const std::string & );
		static std::string	errInviteOnlyChan( const std::string &, const std::string & );
		static std::string	errNoNickNameGiven( void );
		static std::string	errErroneusNickname( void );
		static std::string	errNickNameInUse( void );
		static std::string	errAlreadyRegistred( void );
		static std::string	errNoSuchChannel( const std::string & );
		static std::string	errUserNotInChannel( const std::string &, const std::string & );

		static void			sendRPL( ClientConnection &client, const std::string &content, Server &server);
};

#endif
