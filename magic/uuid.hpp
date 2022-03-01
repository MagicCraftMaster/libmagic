#ifndef UUID_HPP
#define UUID_HPP
#include <iostream>
#include <sstream>
#include <cstdint>
#include <cstring>
#include <string>

namespace {
	using uint64 = std::uint64_t;
}

class UUID {
	uint64 most,least; // 128-bit
public:
	UUID(uint64 most=0, uint64 least=0) : most(most),least(least) {std::cout<<most<<"-"<<least<<std::endl;}
	UUID(std::string str) {UUID ret=from_string(str); most=ret.most; least=ret.least;}

	// UUID: 382037be-741f-41c3-b897-33d6f9edce8c
	static UUID from_string(std::string str) {
		uint64 most=0,least=0;
		std::string dashless;
		for (int i=0,len=str.length(); i<len; i++) if (str[i]!='-') dashless+=std::tolower(str[i]); // Remove hyphens and return all lowercase
		most  = std::stoull(dashless.substr( 0,16), nullptr, 16);
		least = std::stoull(dashless.substr(16,16), nullptr, 16);
		return UUID(most,least);
	}

	std::string to_string(bool upper=1) const {
		std::stringstream ss;
		ss << std::hex << most << std::hex << least;
		std::string out, dashless=ss.str();
		for (int i=0; i<32; i++) {
			out += dashless[i];
			if (i==7 || i==11 || i==15 || i==19) out+='-';
		}
		if (upper) for (int i=0; i<36; i++) out[i] = std::toupper(out[i]);
		return out;
	}

	inline bool operator==(const UUID& b) {return most==b.most && least==b.least;}
	inline bool operator!=(const UUID& b) {return !(*this==b);}

	bool is_zero() const {return most==0 && least==0;}
};

#endif
