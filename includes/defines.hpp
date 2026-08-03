#ifndef DEFINES_HPP
# define DEFINES_HPP

/* Permet d'implementer le make debug */
# ifdef DEBUG_MODE
#  define DEBUG(cmd) do { cmd; } while (0)
# else
#  define DEBUG(cmd) ((void)0)
# endif

# define SUCCESS 0		// le return du main en cas de succes
# define USAGE_ERROR 1  // le return du main en cas derreur d'utilisation
# define MEMORY_ERROR 2 // le return du main en cas d'erreur de memoire

# define MAX_MSG_SIZE 512 //taille max dun message selon norme IRC (/r/n compris)


#endif
