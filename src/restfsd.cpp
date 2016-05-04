#include "../config.h"

#include <thread>
#include <cstring>
#include <sstream>
#include <atomic>
#include <exception>

#include "fcgiapp.h"
#include "fcgio.h"

#include <json/writer.h>
#include <json/reader.h>

#include "object.hpp"
#include "config.hpp"
#include "router.hpp"
#include "error.hpp"


//хранит дескриптор открытого сокета
std::atomic_int socketId;

void GetFile(Router *r) 
{
	User user(r);
	user.login(r->GetInHeader("HTTP_TOKEN"));

	unsigned int id = std::stoul(r->cm[1]);

	File file(r, &user);
	file.Get(id);
}

void PutFile(Router *r) 
{
	User user(r);
	user.login(r->GetInHeader("HTTP_TOKEN"));
	Directory dir(r->db, &user);

	unsigned int id = std::stoul(r->cm[1]);
	
	std::string filename = r->cm[2];
	
	if (id) 	dir.Chdir(id);

	File file(r, &user);
	std::string key  = file.Create(&dir, filename);
	
	r->SetStatus(Httpstatus::Created);
}

void  PostCreateDir(Router *r) 
{
	User user(r);
	user.login(r->GetInHeader("HTTP_TOKEN"));
	
	Directory dir(r->db, &user);
	Json::Value JsonContentIn = r->ParseJsonIn();
	std::string name = JsonContentIn["name"].asString();

	unsigned int id = JsonContentIn["id"].asUInt();
	if (id) 	dir.Create(id,name);
	else dir.Create(name);
	
	r->SetStatus(user.HttpStatus);
	r->AddHeader("Content-Type", user.ContentType);
	r->AddContent("Create dir\r\n");
	
}

void  GetDir(Router *r) 
{
	User user(r);
	user.login(r->GetInHeader("HTTP_TOKEN"));
	
	Directory dir(r->db, &user);
	unsigned int id = std::stoul(r->cm[1]);
	Json::Value result;

	if ( id > 0 ) result = dir.Ls(id);
	else result = dir.Ls();
	
	Json::StyledWriter sw;
	
	r->SetStatus(Httpstatus::OK);
	r->AddHeader("Content-Type", "application/json; charset=utf-8");
	r->AddContent(sw.write(result).c_str());
	
}


void  OptDirs(Router *r) 
{
	
	r->SetStatus(Httpstatus::OK);
}


void  UsersInfo (Router *r) 
{
	User user(r);
	user.login(r->GetInHeader("HTTP_TOKEN"));
	
	
	
	
	r->SetStatus(user.HttpStatus);
	r->AddHeader("Content-Type", user.ContentType);
	r->AddContent("User info\r\n");
}

void OptUsersLogin(Router *r)
{
	r->SetStatus(Httpstatus::OK);
	r->AddHeader("Content-Type", "text/plain");
}

void OptUsersAdd(Router *r)
{
	r->SetStatus(Httpstatus::OK);
}


void PostUsersAdd(Router *r)
{
	User user(r);
	Json::Value JsonContentIn = r->ParseJsonIn();
	user.add(JsonContentIn);
	
	r->SetStatus(user.HttpStatus);
}

void PostUsersLogin(Router *r)
{
	User user(r);
	Json::StyledWriter sw;
	
	Json::Value JsonContentIn = r->ParseJsonIn();
	
	std::string Content = user.login(JsonContentIn);
	
	r->SetStatus(user.HttpStatus);
	r->AddHeader("Content-Type", user.ContentType);
	
	r->AddContent(Content);
	
}


static void *doit(int id, Config &conf)
{
    
    FCGX_Request request;
	
    if(FCGX_InitRequest(&request, socketId.load(), 0) != 0)
    {
        //ошибка при инициализации структуры запроса
        printf("Can not init request\n");
        return NULL;
    }

	Router router(&request, &conf);
	
	router.addHandler("OPTIONS",	"/users/login", 		&OptUsersLogin);
	router.addHandler("GET",		"/users/login", 		&UsersInfo);
	router.addHandler("POST",		"/users/login", 		&PostUsersLogin);
	
	router.addHandler("OPTIONS",	"/users/add", 			&OptUsersAdd);
	router.addHandler("POST",		"/users/add", 			&PostUsersAdd);
	
	router.addHandler("OPTIONS",	".*", 					&OptDirs);
	router.addHandler("OPTIONS",	"/dirs/(?<id>\\d+)",	&OptDirs);
	
	router.addHandler("POST",		"/dirs", 				&PostCreateDir);
	router.addHandler("GET",		"/dirs/(?<id>\\d+)",	&GetDir);
	
	router.addHandler("POST",		"/files/(\\d+)/(.+)",	&PutFile);
	router.addHandler("GET",		"/files/(\\d+)",		&GetFile);


    for(;;)
    {
        static pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER;

        pthread_mutex_lock(&accept_mutex);
        int rc = FCGX_Accept_r(&request);
		
		pthread_mutex_unlock(&accept_mutex);
        
        if(rc < 0)
        {
            //ошибка при получении запроса
            printf("Can not accept new request\n");
            break;
        }

	   	std::streambuf * cin_streambuf  = std::cin.rdbuf();
	    std::streambuf * cout_streambuf = std::cout.rdbuf();
	    std::streambuf * cerr_streambuf = std::cerr.rdbuf();    


        fcgi_streambuf cin_fcgi_streambuf(request.in);
        fcgi_streambuf cout_fcgi_streambuf(request.out);
        fcgi_streambuf cerr_fcgi_streambuf(request.err);

        std::cin.rdbuf(&cin_fcgi_streambuf);
        std::cout.rdbuf(&cout_fcgi_streambuf);
        std::cerr.rdbuf(&cerr_fcgi_streambuf);
		try
		{	
			router.Run();
		}
		catch (Error &e)
		{
			router.SetStatus(e.http_code());
			router.AddHeader("Content-Type", "application/json; charset=utf-8");
			router.AddContent(e.what());
			router.AcceptContent();
		}
		catch (std::exception &e)
		{
			std::cerr << e.what();
			router.SetStatus(Httpstatus::InternalServerError);
			router.AddHeader("Content-Type", "text/plain; charset=utf-8");
			router.AddContent(e.what());
			router.AcceptContent();
		}
		
		FCGX_Finish_r(&request);
		
        //завершающие действия - запись статистики, логгирование ошибок и т.п.

		router.Cleanup();


	    std::cin.rdbuf(cin_streambuf);
	    std::cout.rdbuf(cout_streambuf);
	    std::cerr.rdbuf(cerr_streambuf);

    }


    return NULL;
}

int main(int argc, char **argv)
{
    
    int i;
    
    std::ostringstream conffile;
    conffile << SYSCONFDIR << "/restfsd.ini";
	Config conf(conffile.str());
	
	const int THREAD_COUNT = conf.GetInteger("main","threads",2);
    std::thread *id = new std::thread[THREAD_COUNT];


	//инициализация библилиотеки
	FCGX_Init();

	//открываем новый сокет
	socketId.store( FCGX_OpenSocket(conf.get("main","listen").c_str(), 20) );
	if(socketId < 0)
	{
		//ошибка при открытии сокета
		printf("Socket isn't opened\n");
		return 1;
	}
//    	printf("Socket is opened\n");


        
	//~ int pid;	
	//~ pid = fork();
	//~ if (pid == -1)
	//~ {
		//~ printf("Error: Start Daemon failed (%s)\n", strerror(errno));
		//~ return -1;
	//~ }
	//~ else if (!pid)
	//~ {
		//~ umask(0);
		//~ setsid();
		//~ //chdir("/home/sokol");
		//~ создаём рабочие потоки
		for(i = 0; i < THREAD_COUNT; i++)
		{
			id[i] = std::thread( doit, i, std::ref(conf) );
		}
		//~ sleep(1);
		//~ std::cout << "main1 " <<  conf.get("test","test") << std::endl;
		//ждем завершения рабочих потоков
		//~ for(i = 0; i < THREAD_COUNT; i++)
		//~ {
			//~ id[i].join();
			//~ id[i].detach();
		//~ }
        
        //~ while(true) {
			//~ //std::cout << "while" << std::endl;
	        //~ for(i = 0; i < THREAD_COUNT; i++)
	        //~ {
	            //~ id[i].join();
	            //~ std::cout << "thread[" << i << "].get_id: " << id[i].get_id() << std::endl;
	            //~ if (id[i].joinable())
				//~ {
					//~ id[i].join();
				//~ } else
				//~ {
					//~ std::cout << "thread[" << i << "]: starting" << std::endl;
					//~ id[i] = std::thread( doit, i, std::ref(conf) );
				//~ }
	        //~ }
			//~ //sleep(1);
		//~ }
		//~ close(STDIN_FILENO);
		//~ close(STDOUT_FILENO);
		//~ close(STDERR_FILENO);

		for(i = 0; i < THREAD_COUNT; i++)
		{
			id[i].join();
		}
		
        //~ std::cout << "main2" << conf.get("test","test") << std::endl;
		return 0;

	//~ } else
	//~ {
		//~ printf("Start Daemon pid=(%d)\n", pid);
		//~ return 0;
	//~ }
}

