#ifndef LOGGER_HPP
# define LOGGER_HPP

# include <iostream>
# include <string>

# define COLOR_RESET   "\033[0m"
# define COLOR_WHITE   "\033[37m"
# define COLOR_GREY    "\033[90m"
# define COLOR_CYAN    "\033[36m"
# define COLOR_GREEN   "\033[32m"
# define COLOR_YELLOW  "\033[33m"
# define COLOR_RED     "\033[31m"
# define COLOR_MAGENTA "\033[35m"

# define LOG_TYPES \
    X(DEBUG,     COLOR_GREY,    "[DEBUG]",    std::cout) \
    X(INFO,      COLOR_CYAN,    "[INFO]",     std::cout) \
    X(NOTICE,    COLOR_GREEN,   "[NOTICE]",   std::cout) \
    X(WARNING,   COLOR_YELLOW,  "[WARNING]",  std::cerr) \
    X(USAGE,     COLOR_MAGENTA, "[USAGE]",    std::cerr) \
    X(ERROR,     COLOR_RED,     "[ERROR]",    std::cerr) \
    X(PROTOCOL,  COLOR_YELLOW,  "[PROTOCOL]", std::cerr) \
    X(TRACE,     COLOR_GREY,    "[TRACE]",    std::cout)


enum LogLevel {
# define X(name, color, label, stream) LOG_LEVEL_##name,
    LOG_TYPES
# undef X
};

class Logger {
	private:
	    std::ostream* _stream;

	public:
	    Logger(LogLevel level, const std::string& user = "");
	    ~Logger();

	    template<typename T>
	    Logger& operator<<(const T& val) {
	        (*_stream) << val;
	        return *this;
	    }
};

# define LOG_DEBUG          Logger(LOG_LEVEL_DEBUG)
# define LOG_INFO           Logger(LOG_LEVEL_INFO)
# define LOG_WARN           Logger(LOG_LEVEL_WARNING)
# define LOG_ERR            Logger(LOG_LEVEL_ERROR)
# define LOG_USAGE          Logger(LOG_LEVEL_USAGE)
# define LOG_PROTO          Logger(LOG_LEVEL_PROTOCOL)
# define LOG_TRACE          Logger(LOG_LEVEL_TRACE)

# define LOG_USER_WARN(u)   Logger(LOG_LEVEL_WARNING, u)
# define LOG_USER_ERR(u)    Logger(LOG_LEVEL_ERROR, u)
# define LOG_USER_PROTO(u)  Logger(LOG_LEVEL_PROTOCOL, u)

#endif

