
#ifndef __OBJECTS_HPP__
#define __OBJECTS_HPP__

#include <string>
#include <vector>
#include <map>
#include <exception>


#include <pqxx/pqxx>
#include <libpq-fe.h>

#include <json/writer.h>
#include <json/reader.h>

#include <cryptopp/sha.h>
#include <cryptopp/hex.h>

#include "config.hpp"
#include "httpstatus.hpp"
#include "router.hpp"

class Property 
{
	public:
		virtual std::string GetProperty(std::string name) = 0;
		virtual void SetProperty(std::string name, std::string value) = 0;
};

class Base_Object: public Property
{
	public:
		virtual std::string GetProperty(std::string name) = 0;
		virtual void SetProperty(std::string name, std::string value) = 0;
		std::string sha256(std::string str);
		std::string random_string( size_t length );

};

class User: public Base_Object
{
	public:
		User(Router *_r);
		~User();
		std::string GetProperty(std::string name);
		void SetProperty(std::string name, std::string value);
		std::string login(Json::Value &userdata);
		
		void login(std::string _token);
		unsigned int GetId();
		unsigned int GetDir();
		
		bool add(Json::Value &userdata);
		std::string ContentType;
		Httpstatus HttpStatus;

	private:
		pqxx::connection* db;
		pqxx::result *r;
		unsigned int Id;
		unsigned int root_dirId;
		std::string token;
		Router *router;
		
	
};


class Directory: public Base_Object
{
	public:
		Directory(pqxx::connection *_db, User *_user);
		std::string GetProperty(std::string name);
		void SetProperty(std::string name, std::string value);
		void Chdir(unsigned int _id);
		Directory* Create(std::string name);
		Directory* Create(unsigned int source_dirid, std::string name);
		Json::Value Ls();
		Json::Value Ls(unsigned int source_dirid);
		std::string GetTree();
		
	private:
		pqxx::connection* db;
		pqxx::result *r;
		User *user;
		std::string tree;
		unsigned int Id;
		
		void SetTree();

};

class File: public Base_Object
{
	private:
		pqxx::connection* db;
		pqxx::result *r;
		Router *router;
		User *user;
		ioremap::elliptics::session *elliptics_client;
		unsigned int buflen;

	public:
		File(Router *_r, User *_user);
		std::string GetProperty(std::string name);
		void SetProperty(std::string name, std::string value);
		//void Save();
		std::string Create(Directory *dir, std::string name);
		void Get(unsigned int id);
};


#endif
