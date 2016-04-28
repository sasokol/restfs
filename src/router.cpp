#include "router.hpp"
//#include <regex>

Router::Router(FCGX_Request *_request, Config *_conf)
{
	request = _request;
	conf	= _conf;

	std::ostringstream conninfo;	
	conninfo<< "dbname=" 		<< conf->get("database","dbname")
			<< " host=" 		<< conf->get("database","dbhost")
			<< " port=" 		<< conf->get("database","dbport")
			<< " user=" 		<< conf->get("database","dbuser")
			<< " password=" 	<< conf->get("database","dbpassword");
	
    try
    {
        db = new pqxx::connection(conninfo.str());
    } 
    catch (const pqxx::pqxx_exception &e)
    {
        std::cerr << "Can't connetct to postgres (" << conninfo.str() << ")" << e.base().what() << std::endl;
        //~ const pqxx::sql_error *s=dynamic_cast<const pqxx::sql_error*>(&e.base());
        //~ if (s) std::cerr << "Query was: " << s->query();
        //return NULL;
    }



	ellipcs_logfile = conf->get("elliptics","log", "client.log").c_str();
	int  ellipcs_loglevel = conf->GetInteger("elliptics","log_level", 1);
	ioremap::elliptics::log_level ell_log_level;
	switch (ellipcs_loglevel)
	{
		case 1:
			ell_log_level = DNET_LOG_ERROR;
			break;
		case 2:
			ell_log_level = DNET_LOG_WARNING;
			break;
		case 3:
			ell_log_level = DNET_LOG_INFO;
			break;
		case 4:
			ell_log_level = DNET_LOG_NOTICE;
			break;
		default:
			ell_log_level = DNET_LOG_DEBUG;
			break;	
	}
	
    log = new ioremap::elliptics::file_logger(ellipcs_logfile, ell_log_level);
//    ioremap::elliptics::node el_node;

    n= new ioremap::elliptics::node( ioremap::elliptics::logger(*log, blackhole::log::attributes_t()) );
	
	std::vector<ioremap::elliptics::address> vs = conf->GetAddr("elliptics","backends","127.0.0.1:1026");
    int nodecount = 0;
    std::exception_ptr pe;    
    for (auto it = vs.begin(); it != vs.end(); it++)
    {
		try
		{
			n->add_remote(*it);
			nodecount++;
		}
		catch (const std::exception &e)
		{
			std::cerr << e.what();
			pe = std::current_exception();
		}
	}
	
    if (nodecount == 0) {
        //~ std::rethrow_exception(pe);
    }

    n->set_timeouts(
					conf->GetInteger("elliptics","wait_timeout",50),
					conf->GetInteger("elliptics","ckeck_timeout",50)
				  );

    elliptics_client = new ioremap::elliptics::session(*n);

    elliptics_client->set_groups({ 1, 2 });

	buflen = conf->GetInteger("elliptics","buflen",8193);
	
	Cleanup();

}

void Router::addHandler(std::string method,std::string url, Router::Func handler)
{
	auto m = FuncTable.find(method);
	if (m != FuncTable.end() )
	{
		m->second[url] = handler;
	} 
	else
	{
		Router::RgexUrl pair;
		pair[url] = handler;
		FuncTable[method] = pair;
	}
	
}

std::string Router::MakeKey(std::string method, std::string url)
{
	return method+url;
}

void Router::Run()
{
	
	ParseInHeaders();

	std::string ContentType = GetInHeader("HTTP_CONTENT_TYPE");	
	
	RgexUrl UrlPair = FuncTable[ InHeaders.find("REQUEST_METHOD")->second ];
	Func  func = nullptr;
	
	for (auto it = UrlPair.begin(); it != UrlPair.end(); ++it )
	{
			std::string cstr = InHeaders.find("SCRIPT_NAME")->second;
			std::string regex = it->first;
			boost::regex e (regex);
			
			if ( boost::regex_match ( cstr.c_str(), cm, e, boost::regex_constants::match_default) )
			{
				func = it->second;

				break;

			}

	}
	
	if (func != nullptr) 
	{
		func(this, request);
	} else
	{
		DefaultHandler();
	}

	//AcceptHeaders();
	AcceptContent();

	//Cleanup();
}

void Router::ParseInHeaders()
{
	for(int i = 0; request->envp[i] != NULL; i++ )
	{
		std::vector<std::string> vs;
		std::string text = std::move(request->envp[i]);
		boost::split(vs,text,boost::is_any_of("="));
		InHeaders.emplace(
				vs[0],
				FCGX_GetParam(vs[0].c_str(), request->envp)
		);
	}

}

std::multimap<std::string,std::string> Router::GetInHeaders()
{
	return InHeaders;
}

void Router::Cleanup() 
{
	InHeaders.clear();
	OutHeaders.clear();
	ContentOut = "";
	
	HeadersAccepted = false;
	ContentAccepted = false;
	//JsonContentIn.clear();

	AddHeader("Access-Control-Allow-Origin", "*");
	AddHeader("Access-Control-Allow-Credentials", "true");
//	AddHeader("Access-Control-Allow-Headers", "X-Requested-With, content-type, accept, accept-language, content-type, token");
	AddHeader("Access-Control-Allow-Headers", "X-Requested-With,content-type,token");
	AddHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS,PUT,PATCH,DELETE");
	
}

void Router::DefaultHandler()
{
	std::cerr << "The method is not allowed";
	SetStatus(Httpstatus::MethodNotAllowed);
	AddHeader("Content-length", 0);
}

void Router::AcceptHeaders()
{
	if (!HeadersAccepted) {
		for(auto it: OutHeaders)
		{
			FCGX_PutS((it.first + ": " + it.second + "\r\n").c_str(), request->out);
		}
		FCGX_PutS("\r\n", request->out);
		HeadersAccepted = true;
	}
}

void Router::AcceptContent()
{
	if (!ContentAccepted) {
		if (!HeadersAccepted) {
			AddHeader("Content-Length", ContentOut.size());
			AcceptHeaders();
		}
		FCGX_PutS(ContentOut.c_str(), request->out);
		ContentAccepted = true;
	}
}


void Router::AddHeader(std::string header,std::string value)
{
	OutHeaders[header] = value;
}

void Router::AddHeader(std::string header,long int value)
{
	AddHeader(header,std::to_string(value));
}

void Router::SetStatus(Httpstatus value)
{
	AddHeader("Status",std::to_string((unsigned int)value));
}

std::string Router::GetInHeader(std::string header)
{
	auto it =  InHeaders.find(header);
	if (it != InHeaders.end())
		return it->second;
	return "";
}

long int Router::GetInHeaderInt(std::string header)
{
    std::string valstr = GetInHeader(header);
    const char* value = valstr.c_str();
    char* end;
    // This parses "1234" (decimal) and also "0x4D2" (hex)
    long n = strtol(value, &end, 0);
    return end > value ? n : 0;

}

void Router::AddContent(std::string part)
{
	ContentOut +=part;
}

void Router::ContentManualy()
{
	ContentAccepted = true;
}

Json::Value Router::ParseJsonIn()
{

	std::string ContentType = GetInHeader("HTTP_CONTENT_TYPE");	
	if (ContentType != "application/json") 
	{
		std::cerr << "Content-Type isn't application/json";
		SetStatus(Httpstatus::BadRequest);		
//		return;
	}
	
	long int ContentLength = GetInHeaderInt("HTTP_CONTENT_LENGTH");
	if (ContentLength > 20480) 
	{
		std::cerr << "Request Entity Too Large";
		SetStatus(Httpstatus::RequestEntityTooLarge);
//		return;
	}

	std::string Content = "";
	std::unique_ptr<char[]> post{ new char [ContentLength]};
	
	if (ContentLength > 0) 
	{
		FCGX_GetStr(post.get(), ContentLength, request->in);
		Content = post.get();	
	}
	
	Json::Reader reader;
	Json::Value JsonContentIn;
	bool parsingSuccessful = reader.parse( Content, JsonContentIn );
	if ( !parsingSuccessful )
	{
	    // report to the user the failure and their locations in the document.
	    std::cerr  << "Failed to parse configuration\n"
	               << reader.getFormattedErrorMessages();
	    SetStatus(Httpstatus::BadRequest);
//	    return;
	}
	return JsonContentIn;

}
