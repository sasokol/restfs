#include "config.hpp"

#include <cstring>
#include <iostream>
#include <sstream>
#include <fstream>
#include "error.hpp"


Config::Config(std::string filename)
{
	DefaultParams();
	if (std::ifstream(filename) == NULL) throw Error(Errors::CONFIG_NOT_FOUND, filename + " not found");
	
	_error = ini_parse(filename.c_str(), ValueHandler, this);
}

Config::Config()
{
	DefaultParams();
	//std::cerr << "DEBUG: start()" << std::endl;
}


Config::~Config()
{
	//std::cout << GetData();
}

std::string Config::get(std::string section, std::string key, std::string v_default) const
{
	std::string res = get(section, key);
	return res.size() ? res : v_default; 
}

std::string Config::get(std::string section, std::string key) const
{
	auto sec = _values.find(section);
	if ( sec != _values.end() )
	{
		auto name = sec->second.find(key);
		if (name != sec->second.end() ) 
			return name->second;
	} 
	return "";
}

void Config::set(std::string section, std::string key, std::string value)
{
    m.lock();
    auto sec = _values.find(section);
    if ( sec != _values.end() )
    {
		sec->second[key] = value;
	} else
    {
		Pair pair;
		pair[key] = value;
		_values[section] = pair;
	}
	m.unlock();

}

void Config::DefaultParams()
{
/*
	iniDefault.SetUnicode();
	iniDefault.SetValue("main", 		NULL, 			NULL,				"# Basic params");
	iniDefault.SetValue("main", 		"listen", 		"127.0.0.1:9000", 	"# listen FCGI host:port. Default is 127.0.0.1:9000");
	
	iniDefault.SetValue("database", 	NULL, 			NULL,				"# Params for connect to Postgres database");
	iniDefault.SetValue("database", 	"dbhost", 		"127.0.0.1", 		"# Host name or IP for connect to database. Default is localhost");
	iniDefault.SetValue("database", 	"dbport", 		"5432", 			"# Port for connect to Postgres. Default is 5432");
	iniDefault.SetValue("database", 	"dbuser", 		"restfs", 			"# Data base name. Default is restfs.");
	iniDefault.SetValue("database", 	"dbname", 		"restfs", 			"# Data base user. Default is restfs.");
	iniDefault.SetValue("database", 	"dbpass", 		"", 				"# Data base password. Default is empty.");
	
	iniDefault.SetValue("elliptics", 	NULL, 			NULL, 				"# Params for connect to elliptics storage.");
	iniDefault.SetValue("elliptics", 	"backends", 	"localhost:1026", 	"# List host:post. Default is localhost:1026");
*/	

	set("main", 		"listen", 		"127.0.0.1:9000");
	set("main", 		"threads", 		"2");
	
	set("database", 	"dbhost", 		"127.0.0.1" 	);
	set("database", 	"dbport", 		"5432"	 		);
	set("database", 	"dbuser", 		"restfs" 		);	
	set("database", 	"dbname", 		"restfs" 		);
	set("database", 	"dbpass", 		""  			);
	
	set("elliptics", 	"backends", 	"127.0.0.1:1026");
	set("elliptics", 	"log", 			"elliptics.log");
	set("elliptics", 	"log_level", 	"1");
	set("elliptics", 	"buflen", 		"23522"			);
	set("elliptics", 	"wait_timeout",	"50"			);
	set("elliptics", 	"check_timeout","50"			);

}

void Config::SaveToFile(std::string file)
{
		SaveFile(file);
}

int Config::ValueHandler(void* user, const char* section, const char* name,
                            const char* value)
{
    Config* reader = (Config*)user;
    reader->set(section,name,value);
    return 1;
}

std::string Config::GetData() 
{
	std::ostringstream data;
	for (auto it = _values.begin(); it != _values.end(); ++it)  ///вывод на экран
	{
		data << "[" << it->first << "]" << std::endl;
		auto _pair = it->second;
		for (auto jt = _pair.begin(); jt != _pair.end(); ++jt)
		{
			data << jt->first << " = " << jt->second << std::endl;
		}
	}
	return data.str();
}

void Config::SaveFile(std::string file) {
	std::ofstream myfile(file);
	myfile << GetData();
	myfile.close();
}

std::vector<std::string> Config::GetArray(std::string section, std::string name, std::string default_value) const
{
	std::vector<std::string> vs;
    std::string text = get(section, name, default_value);
    boost::split(vs,text,boost::is_any_of(", "));
    return vs;
}

std::vector<std::string> Config::GetArray(std::string section, std::string name) const
{
	return GetArray(section, name, "");
}

std::vector<ioremap::elliptics::address> Config::GetAddr(std::string section, std::string name) const
{
	return GetAddr(section,name,"127.0.0.1:1026");
}

std::vector<ioremap::elliptics::address> Config::GetAddr(std::string section, std::string name, std::string default_value) const
{
	std::vector<std::string> vs = GetArray(section, name, default_value);
	std::vector<ioremap::elliptics::address> result;
	for (auto it = vs.begin(); it != vs.end(); it++)
	{
	    std::string text = *it;
	    boost::trim(text);
	    if (text == "")
			continue;

	    std::vector<std::string> pair;
		boost::split(pair,text,boost::is_any_of(":"));
		const char* value = pair[1].c_str();
		char* end;
		int port = strtol(value, &end, 0);
		ioremap::elliptics::address addr(pair[0].c_str(),port);
		result.push_back(addr);
	}
	return result;
}

long Config::GetInteger(std::string section, std::string name, long default_value) const
{
    std::string valstr = get(section, name, "");
    const char* value = valstr.c_str();
    char* end;
    // This parses "1234" (decimal) and also "0x4D2" (hex)
    long n = strtol(value, &end, 0);
    return end > value ? n : default_value;
}
