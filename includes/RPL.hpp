#ifndef RPL_HPP
# define RPL_HPP
# include "header.hpp"

// /connect DALNET + /RAWLOG OPEN debug.log for debug logs on official server (to test display)

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

		static std::string	rplNoTopic(const std::string &, const std::string &);
		static std::string	rplTopic(const std::string &, const std::string &,  const std::string &);

		static std::string	errNeedMoreParams(const std::string &command);
		static std::string	errNotOnChannel(const std::string &command);
		static std::string	errChanOpPrivsNeeded(const std::string &, const std::string &);
		static std::string	errNoChanModes(const std::string &command);
};

#endif
