

#ifndef __ROUTER_HPP__
#define __ROUTER_HPP__

#include <string>
#include <functional>
#include <map>

//#include <regex>
#include <boost/regex.hpp>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>


#include "fcgiapp.h"
#include "fcgio.h"

#include <json/writer.h>
#include <json/reader.h>

#include <pqxx/pqxx>
#include <libpq-fe.h>

#include <elliptics/session.hpp>

#include "config.hpp"
#include "httpstatus.hpp"


class Router 
{
	public:
		typedef std::function< void(Router *me, FCGX_Request *_request)> Func;
		typedef std::map<std::string, Router::Func> RgexUrl;

		pqxx::connection* db;
		unsigned int buflen;
		ioremap::elliptics::session *elliptics_client;
		//ioremap::elliptics::node n;
		
		Router(FCGX_Request *_request, Config *_conf);
		
		void addHandler(std::string method,std::string url, Router::Func handler);
		void Run();
		
		//std::map<std::string, Router::RgexUrl> GetFuncTable();
		std::multimap<std::string,std::string> GetInHeaders();

		void AddHeader(std::string header,std::string value);
		void AddHeader(std::string header,long int value);
		//void AddHeader(std::string header,Httpstatus value);
		void SetStatus(Httpstatus value);
		
		void AddContent(std::string part);
		std::string GetInHeader(std::string header);
		long int GetInHeaderInt(std::string header);
		void AcceptHeaders();
		void AcceptContent();
		boost::cmatch cm; 
		
		Json::Value ParseJsonIn();
		void ContentManualy();
		void Cleanup();
		FCGX_Request *request;
	
	private:

		Config *conf;
	
		std::map<std::string, Router::RgexUrl> FuncTable;
		std::multimap<std::string,std::string> InHeaders;
		std::map<std::string,std::string> OutHeaders;
		
		bool HeadersAccepted;
		bool ContentAccepted;
		
		std::string ContentOut;
		
		std::string MakeKey(std::string method, std::string url);
		void DefaultHandler();
		void ParseInHeaders();
		
		const char *ellipcs_logfile;
		ioremap::elliptics::file_logger *log;
		ioremap::elliptics::node *n;
};

#endif
