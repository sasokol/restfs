#include "error.hpp"


Error::Error(Errors error)
	: errorcode(error)
{
	ErrorObject["code"] = (unsigned int)errorcode;
	ErrorObject["message"] = messages[error].message;
}

Error::Error(Errors error, std::string message)
	: errorcode(error)
{
	ErrorObject["code"] = (unsigned int)errorcode;
	ErrorObject["message"] = messages[error].message;
	ErrorObject["ext"] = message;
}


const char* Error::what() const throw()
{
	Json::StyledWriter sw;
	
	return  sw.write(ErrorObject).c_str();
}

Httpstatus Error::http_code()
{
	return messages[ errorcode ].status;
}
