#ifndef __ERROR_HPP__
#define __ERROR_HPP__

#include <string>
#include <vector>
#include <map>
#include <exception>

#include <json/writer.h>
#include <json/reader.h>

#include "httpstatus.hpp"

enum class Errors
{
	EMAIL_EXIST 	= 101,
	EMAIL_NDEFINED 	= 102,
	PASS_NDEFINED 	= 103,
	AUTH_FALED		= 104,
	TOKEN_INVALID	= 105,
	NOT_FOUND		= 106,
	OBJECT_EXIST	= 107,
	CTYPE_NDEFINED	= 108,
	CLENGTH_NDEFINED= 109,
	CONFIG_NOT_FOUND= 110,
	ID_NDEFINED		= 111,
	DIR_NOTEMPTY	= 112,
	
	UNKNOWN_ERROR 	= 500
};


class Error: public std::exception
{
		public:
			Error(Errors error);
			Error(Errors error, std::string message);
			virtual const char* what() const throw();
			Httpstatus http_code();
		
		private:
			Errors errorcode;
			struct ErrorMessage {
				std::string message;
				Httpstatus status;
			};
			Json::Value ErrorObject;

			std::map<Errors, ErrorMessage > messages = {
				{Errors::EMAIL_EXIST, 		{"Email is exist", 					Httpstatus::Conflict  } },
				{Errors::EMAIL_NDEFINED, 	{"Email is not defined", 			Httpstatus::BadRequest} },
				{Errors::PASS_NDEFINED, 	{"Password is not defined", 		Httpstatus::BadRequest} },
				{Errors::AUTH_FALED, 		{"Username or password incorrect", 	Httpstatus::Unauthorized} },
				{Errors::TOKEN_INVALID, 	{"Token is not valid",		 		Httpstatus::Unauthorized} },
				{Errors::NOT_FOUND, 		{"Object not found",		 		Httpstatus::NotFound} },
				{Errors::OBJECT_EXIST, 	{"Object is exist", 				Httpstatus::Conflict  } },
				{Errors::CTYPE_NDEFINED, 	{"Content-Type is not defined", 	Httpstatus::UnsupportedMediaType } },
				{Errors::CLENGTH_NDEFINED, {"Content-Length is not defined", 	Httpstatus::LengthRequired } },
				{Errors::CONFIG_NOT_FOUND, {"Config file not found", 			Httpstatus::InternalServerError } },
				{Errors::ID_NDEFINED, 		{"ID is not defined", 				Httpstatus::BadRequest} },
				{Errors::DIR_NOTEMPTY, 	{"Directory isn't empty",			Httpstatus::Conflict} },
				
				{Errors::UNKNOWN_ERROR, 	{"Unknown error", 					Httpstatus::InternalServerError} }
			};			
};

#endif


