#ifndef MAGIC_STREAM_LOGGER_BUFFER_HPP
#define MAGIC_STREAM_LOGGER_BUFFER_HPP
#include <algorithm>
#include <functional>
#include <optional>
#include <cstdint>
#include <ostream>
#include <iostream>
#include <string>
#include <vector>
#include "magic/logging/log_level.hpp"

#if defined( __linux__ ) || defined( __APPLE__ )
#define COLOR_SUPPORTED true
#else
#define COLOR_SUPPORTED false
#endif

namespace {
	using byte = std::uint8_t;
}

std::ostream& operator<<(std::ostream& os, const magic::LogLevel& level);
std::ostream& operator<<(std::ostream& os, const magic::LogSpecial& special);

namespace magic {
	class StreamLoggerBuffer : public std::streambuf {
		static void default_push_header(std::ostream& stream, const std::string& stream_name, LogLevel level, bool colored) {
			const std::string level_name = LogLevelHelpers::get_level_name(level);
			const std::string color      = !colored ? "" : LogLevelHelpers::get_level_color(level);
			const std::string reset      = !colored ? "" : "\033[0m";
			const bool exsp = level==LogLevel::INFO || level==LogLevel::WARN;
			// [name] [Level] *
			stream << reset << "["<<stream_name<<"] [" << color << level_name << reset << (exsp ? "]  " : "] ") << color;
		}

		union {
			byte levels;
			struct {
				byte error:1;
				byte warn:1;
				byte info:1;
				byte debug:1;
				byte debex:1;
				byte color:1;
				byte headless:1;
				byte spanning:1;
			} flags;
		};
		std::string name;
		std::vector<std::reference_wrapper<std::ostream>> inputs; // This should only ever have one value but in case it doesn't this will ensure that all paired streams are handled
		std::vector<std::reference_wrapper<std::ostream>> outputs;
		std::function<void(std::ostream&,const std::string&,LogLevel,bool)> push_header_func;
		byte last_level=5; // Invalid to say no level

		short get_output( std::ostream& stream ) const {
			for (short i=0,len=outputs.size(); i<len; i++) {
				if (&outputs[i].get() == &stream)
					return i;
			}
			return -1;
		}
	public:
		static bool is_color_supported() {return COLOR_SUPPORTED;}

		static void exec_stream_safe( std::ostream& os, std::function<void(StreamLoggerBuffer& buffer)> func, std::function<void()> errfunc=[](){} ) {
			std::streambuf &buffer = *os.rdbuf();
			try {
				magic::StreamLoggerBuffer &lbuffer = dynamic_cast<magic::StreamLoggerBuffer&>(buffer);
				func(lbuffer);
			} catch (const std::bad_cast &err) {
				errfunc();
			}
		}

		StreamLoggerBuffer(std::string name, LogLevel top_level=LogLevel::INFO) : name(name), outputs{std::cout}, push_header_func(StreamLoggerBuffer::default_push_header) {
			set_top_level( top_level );
			flags.color = true;

			for (std::ostream& stream : outputs) {
				push_header( stream, LogLevel::DEBEX );
				push_special( stream, LogSpecial::START_SPAN );
				stream << "\nCreated StreamLoggerBuffer:"
						<< "\n\tName: " << name
						<< "\n\tFlags:"
						<< "\n\t\terror: " << (flags.error?"true":"false")
						<< "\n\t\twarn: "  << (flags.warn ?"true":"false")
						<< "\n\t\tinfo: "  << (flags.info ?"true":"false")
						<< "\n\t\tdebug: " << (flags.debug?"true":"false")
						<< "\n\t\tdebex: " << (flags.debex?"true":"false")
						<< "\n\t\tcolor: " << (flags.color?"true":"false")
						<< (is_color_supported()&&flags.color ? "\033[0m" : "")
						<< std::endl;
				push_special( stream, LogSpecial::END_SPAN );
			}
		}

		void add_output    (std::ostream& stream) {if (get_output(stream) == -1) outputs.push_back(stream);}
		void remove_output (std::ostream& stream) {short idx=get_output(stream); if (idx!=-1) outputs.erase( outputs.begin()+idx );}

		void add_outputs    (std::vector<std::reference_wrapper<std::ostream>> streams) { for ( std::ostream& stream : streams ) add_output( stream ); }
		void remove_outputs (std::vector<std::reference_wrapper<std::ostream>> streams) { for ( std::ostream& stream : streams ) remove_output( stream ); }

		void set_top_level(LogLevel level) {levels = (levels&~31) | ((2<<(byte)level)-1);}

		void set_level_enabled(LogLevel level, bool b) { byte bit=(1<<(byte)level); levels = (levels&~bit) | (b ? bit : 0); }
		void set_levels_enabled(std::vector<LogLevel> levels, bool b) {
			byte bits = 0;
			for (LogLevel level : levels)
				bits |= 1<<(byte)level;
			this->levels = (this->levels&~bits) | (b ? bits : 0);
		}

		void set_color_enabled(bool b) {flags.color = b;}
		bool is_color_enabled() const {return flags.color;}

		void set_header_func( std::function<void(std::ostream&,const std::string&,LogLevel,bool)> push_header_func ) {this->push_header_func = push_header_func;}

		void push_header( std::ostream& os, const LogLevel& level ) {
			pair( os );
			if (push_header_func == nullptr)
				set_header_func( default_push_header );
			if (!flags.headless)
				push_header_func( os, name, level, is_color_supported() && is_color_enabled() );
			else
				flags.headless = false;
		}

		void push_special( std::ostream& os, const LogSpecial& special ) {
			pair( os );
			switch (special) {
				case LogSpecial::HEADLESS:   flags.headless = true; break;
				case LogSpecial::START_SPAN: flags.spanning = true; break;
				case LogSpecial::END_SPAN:   flags.spanning = false; break;
			}
		}

	protected:
		void pair( std::ostream& os ) {
			exec_stream_safe(os, [&](StreamLoggerBuffer& _){
				for (std::ostream& input : inputs)
					if ( &input == &os )
						return;
				inputs.push_back( os );
			});
		}

		bool is_level_enabled( LogLevel level ) {return levels&(1<<(byte)level);}

		virtual int_type overflow(int_type chr) override {
			bool blocked = last_level<=4 ? !is_level_enabled( (LogLevel)last_level ) : false;
			int_type chrr = blocked ? 0 : chr;
			if ( chr == '\n' && !flags.spanning ) {
				last_level = 5;
				if (is_color_supported() && is_color_enabled()) {
					// Force clear color
					for (std::ostream& input : inputs)
						input << "\033[0m";
				}
			}
			for (std::ostream& stream : outputs)
				stream.rdbuf()->sputc(chrr);
			return chrr;
		}
	};
}

std::ostream& operator<<(std::ostream& os, const magic::LogLevel& level) {
	magic::StreamLoggerBuffer::exec_stream_safe(os, [&](magic::StreamLoggerBuffer& buffer){
		buffer.push_header( os, level );
	}, [](){
		std::cerr << "Attempted to push LogLevel to a standard std::ostream without a StreamLoggerBuffer" << std::endl;
	});
	return os;
}

std::ostream& operator<<(std::ostream& os, const magic::LogSpecial& special) {
	magic::StreamLoggerBuffer::exec_stream_safe(os, [&](magic::StreamLoggerBuffer& buffer){
		buffer.push_special( os, special );
	}, [](){
		std::cerr << "Attempted to push LogSpecial to a standard std::ostream without a StreamLoggerBuffer" << std::endl;
	});
	return os;
}

#endif
