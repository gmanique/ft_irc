#include "ft_irc.hpp"

int ft_strlen(const char *s)
{
    const char *p = s;
    if (!s) return 0;
    while (*p++) ;
    return (size_t)(p - s - 1);
}

uint8_t	isPort(char *s) {
	if (ft_strlen(s) != 4 || s[0] != '6' || s[1] != '6' || s[2] != '6'
		|| s[3] < '5' || s[3] > '9')
		return (0);
	return (1);
}


int main(int ac, char **av)
{
	if (ac != 3 || !isPort(av[1]))
	{
		LOG_USAGE << "Ircserv needs 2 arguments, respectively the port (between 6665 and 6669) and the password.\nExample : `./ircserv 6667 mySafePassword`\n";
		return (USAGE_ERROR);
	}
	Server test(6660 + (av[1][3] - '0'), av[2]);
	int	init_res = test.init();
	if (init_res)
		return (init_res);
	return (SUCCESS);
}


/* main de test pour le logger */
/*
int main()
{
	LOG_DEBUG << "Message de debug";
	LOG_INFO << "Message de info";
	LOG_WARN << "Message de warning";
	LOG_ERR << "Message de error";
	LOG_USAGE << "Message de usage";
	LOG_PROTO << "Message de proto";
	LOG_USER_WARN("Username") << "Message de warn user";
	LOG_USER_ERR("Roberto") << "Message de error user";
	LOG_USER_PROTO("Roberta") << "Message de proto user";
	return (0);
}
*/
