
#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <elliptics/session.hpp>

#include "inih/ini.h"

class Config
{
	public:
		Config(std::string filename);
		Config();
		~Config();
		std::string get(std::string section, std::string key, std::string v_default) const;
		std::string get(std::string section, std::string key) const;
		
		long GetInteger(std::string section, std::string name, long default_value) const;
		long GetInteger(std::string section, std::string name) const;
		
		std::vector<std::string> GetArray(std::string section, std::string name, std::string default_value) const;
		std::vector<std::string> GetArray(std::string section, std::string name) const;
		
		std::vector<ioremap::elliptics::address> GetAddr(std::string section, std::string name, std::string default_value) const;
		std::vector<ioremap::elliptics::address> GetAddr(std::string section, std::string name) const;
		
		void set(std::string section, std::string key, std::string value);
		void SaveToFile(std::string file);

	private:
		int _error;
		std::string inifile;
		std::mutex  m;
		
		typedef std::map<std::string, std::string> Pair;
		std::map<std::string, Pair> _values;
	
		void DefaultParams();
		std::string GetData();
		void SaveFile(std::string file);
		static int ValueHandler(void* user, const char* section, const char* name,
								const char* value);
	
			
};

#endif
