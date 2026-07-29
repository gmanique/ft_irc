#ifndef FT_IRC_HPP
# define FT_IRC_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>

#include <cerrno>
#include <cstdlib>

/* je sais pas si y'en a besoin
#include <dirent.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/wait.h>
*/


/* Permet d'implementer le make debug */
# ifdef DEBUG_MODE
#  define DEBUG(cmd) do { cmd; } while (0)
# else
#  define DEBUG(cmd) ((void)0)
# endif

#include "Server.hpp"

#endif
