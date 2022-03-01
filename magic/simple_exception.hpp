#ifndef LIBMAGIC_SIMPLE_EXCEPTION
#define LIBMAGIC_SIMPLE_EXCEPTION

#include <exception>
#include <string>
#include <cstdint>

namespace magic {
	class simple_exception : public std::exception {
		std::string err;
		uint8_t code;
	public:
		simple_exception(uint8_t code, std::string err) : err(err), code(code) {}
		simple_exception(              std::string err) : err(err), code(127) {} // -1
		
		const char* what() const noexcept override {return err.c_str();}
		uint8_t value() const {return code;}
	};
}

#endif
