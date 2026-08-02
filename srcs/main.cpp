#include "ft_irc.hpp"

int ft_strlen(const char *s)
{
    const char *p = s;
    if (!s) return 0;
    while (*p++) ;
    return (size_t)(p - s - 1);
}

// Potentiellement changer et verifier que c'est entre 6665 et 6669 (un port IRC)
// Je sais pas encore si on doit prendre n'importe quel port ou juste port IRC
uint8_t isValidPort(char *arg) {
	const std::string str(arg);
	if (str.length() > 5 || str.length() == 0)
		return (0);
    for (size_t i = 0; i < str.length(); ++i) {
        if (!std::isdigit(str[i]))
            return false;
    }
    int port = std::atoi(str.c_str());
    return (port >= 1024 && port <= 65535);
}

int main(int ac, char **av)
{
	if (ac != 3 || !isValidPort(av[1]))
	{
		LOG_USAGE << "Ircserv needs 2 arguments, respectively the port (between 6665 and 6669) and the password.\nExample : `./ircserv 6667 mySafePassword`\n";
		return (USAGE_ERROR);
	}
	int port = std::atoi(av[1]);
	Server irc(port, av[2]);
	int	init_res = irc.init();
	if (init_res)
		return (init_res);
	
	int	serv_res = irc.run();
	if (serv_res)
		return (serv_res);

	return (SUCCESS);
}


