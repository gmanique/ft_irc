#include "Logger.hpp"

Logger::Logger(LogLevel level, const std::string &user) : _stream(&std::cout) {
	switch (level) {
	# define X(name, color, label, stream) \
		case LOG_LEVEL_##name: \
			_stream = &stream; \
			(*_stream) << color << label << COLOR_WHITE; \
			if (!user.empty()) { \
				(*_stream) << " [" << COLOR_CYAN << user << COLOR_WHITE << "]"; \
			}	\
			(*_stream) << " : "; \
			break;
        LOG_TYPES
	# undef X
	}
}

Logger::~Logger() {
	(*_stream) << COLOR_RESET << std::endl;
}


