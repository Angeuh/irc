 #include "../includes/Server.hpp"

std::vector<std::string> split( const std::string& str ) {
	std::vector<std::string>	res;
	size_t 						start = 0;
	size_t						end = str.find(',');

	while (end != std::string::npos) {
		res.push_back(str.substr(start, end - start));
		start = end + 1;
		end = str.find(',', start);
	}
	res.push_back(str.substr(start));
	return (res);
}


int	isNicknameAvailable( std::map<int, ClientConnection> &clients, std::string &nick )
{
	for (std::map<int, ClientConnection>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->second.username == nick && it->second.isRegistered == true)
            return (FAILURE);
    }
	return (SUCCESS);
}

int	verifNickname( std::string &nick )
{
	//to do
	(void) nick;
    std::string special = "[]\\`_^{}|";
	if (nick.size() > 9 || nick.size() < 1)
        return (FAILURE);

    char first = nick[0];
    if (!std::isalpha(first) && !isSpecial(first))
        return (FAILURE);

    for (size_t i = 1; i < nick.length(); i++) 
    {
        char c = nick[i];

        if (!std::isalnum(c) &&!isSpecial(c) && c != '-') 
            return (FAILURE);
    }
	return (SUCCESS);
}

std::string itoa(int n)
{
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

bool isSpecial(char c) 
{
    std::string special = "[]\\`_^{}|";
    return special.find(c) != std::string::npos;
}
