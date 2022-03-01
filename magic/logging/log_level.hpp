#ifndef MAGIC_LOG_LEVEL_HPP
#define MAGIC_LOG_LEVEL_HPP
#include <cstdint>
#include <string>

namespace magic {
	enum class LogLevel : std::uint8_t { ERROR, WARN, INFO, DEBUG, DEBEX };

	namespace LogLevelHelpers {
		std::string get_level_name(LogLevel level) {
			switch (level) {
				case LogLevel::ERROR: return "Error";
				case LogLevel::WARN:  return "Warn";
				case LogLevel::INFO:  return "Info";
				case LogLevel::DEBUG: return "Debug";
				case LogLevel::DEBEX: return "Debex";
			}
			throw -1; // impossible condition met
		}
		std::string get_level_color(LogLevel level) {
			switch (level) {
				case LogLevel::ERROR: return "\033[31m";
				case LogLevel::WARN:  return "\033[33m";
				case LogLevel::INFO:  return "\033[36m";
				case LogLevel::DEBUG: return "\033[32m";
				case LogLevel::DEBEX: return "\033[35m";
			}
			throw -1; // impossible condition met
		}
	}

	enum class LogSpecial : std::uint8_t { HEADLESS, START_SPAN, END_SPAN };
}

#endif
