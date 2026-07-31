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
		LOG_USAGE << "ft_irc needs 2 arguments, respectively the port (between 6665 and 6669) and the password.";
		return (USAGE_ERROR);
	}
	Server test(6660 + (av[1][3] - '0'), av[2]);

	return (SUCCESS);
}
