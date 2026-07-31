#ifndef DEFINES_HPP
# define DEFINES_HPP

# define BLACK "\033[0;30m"
# define GREEN "\033[0;32m"
# define RED "\033[0;31m"
# define YELLOW "\033[0;33m"
# define BLUE "\033[0;34m"
# define CYAN "\033[0;36m"
# define PURPLE "\033[0;35m"
# define WHITE "\033[0;37m"

# define LOG_ERROR(error) \
	std::cerr << RED << "[ERROR] : " \
	<< WHITE << error << std::endl;

# define LOG_USER_ERROR(user, error) \
	std::cerr << RED << "[ERROR] " << WHITE << ": " \
	<< YELLOW << user \
	<< WHITE << error << std::endl;

/* Permet d'implementer le make debug */
# ifdef DEBUG_MODE
#  define DEBUG(cmd) do { cmd; } while (0)
# else
#  define DEBUG(cmd) ((void)0)
# endif

# define SUCCESS 0		// le return du main en cas de succes
# define USAGE_ERROR 1  // le return du main en cas derreur d'utilisation
# define MEMORY_ERROR 2 // le return du main en cas d'erreur de memoire


/*
# undef BLACK
# undef GREEN
# undef RED
# undef YELLOW
# undef BLUE
# undef CYAN
# undef PURPLE
# undef WHITE
*/

#endif
