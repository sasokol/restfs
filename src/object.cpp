#include "object.hpp"
#include "error.hpp"

File::File(Router *_r, User *_user) 
{
	router = _r;
	db = router->db;
	user = _user;
	elliptics_client = router->elliptics_client;
	buflen = router->buflen; 
}

void File::Get(unsigned int id)
{
	pqxx::work w(*db);
	r = new pqxx::result( w.exec("SELECT key,size,ctype,name from objects where id=" + std::to_string(id) + " and not is_dir and userid=" + std::to_string(user->GetId() ) ) );
	if (r->empty()) throw Error(Errors::NOT_FOUND);
	
	auto row = r->begin();
	
	std::string key = row["key"].as<std::string>();
	unsigned int file_size = row["size"].as<unsigned int>();
	unsigned int total = file_size;
	
	//~ auto lookup_result = elliptics_client->lookup(key);
	//~ lookup_result.wait();

	uint64_t len = buflen;
	uint64_t offset = 0;
	bool first = true;
	while (file_size > 0 )
	{
		len = ( len > file_size ? file_size : buflen );
		file_size -= len;
		auto async_result = std::move(elliptics_client->read_data(key, offset, len));
		offset += len;
		async_result.wait();
		
		if (first) {
			router->AddHeader("Content-length", total);
			router->AddHeader("Content-type", row["ctype"].as<std::string>());
			router->SetStatus(Httpstatus::OK);
			router->AcceptHeaders();
			router->ContentManualy();
			first = false;
		}
		
		for (auto entry : async_result.get() ) {
			if (entry.status() == 0) {
				auto file = std::move(entry.file());
				FCGX_PutStr(reinterpret_cast<const char *>(file.data()), len, router->request->out);
				break;
			}
		}
	}

	w.commit();
}

std::string File::Create(Directory *dir, std::string name)
{

	unsigned long total = router->GetInHeaderInt("CONTENT_LENGTH");
	std::string ctype = router->GetInHeader("CONTENT_TYPE");
	auto content_length = total;

	if (ctype == "") throw Error(Errors::CTYPE_NDEFINED);
	if (content_length <= 0) throw Error(Errors::CTYPE_NDEFINED);


	pqxx::work w(*db);
	r = new pqxx::result(w.exec("SELECT 1 from objects where tree ~ '" + dir->GetTree() + ".*{1}' and name="+w.quote(name) + " and userid=" + std::to_string(user->GetId())));
	if (!r->empty()) throw Error(Errors::OBJECT_EXIST);
	std::string key = "";
	do 
	{
		key = random_string(40);
		r = new pqxx::result(w.exec("SELECT 1 FROM objects where key=" + w.quote(key) ));
	} while ( !r->empty() );
	
	r = new pqxx::result(w.exec("SELECT nextval('objects_id_seq'::regclass)"));
	auto row = r->begin();
	unsigned int file_id = row["nextval"].as<unsigned int>();
	std::string new_tree = dir->GetTree() + "." + std::to_string(file_id);
	
    auto async_result = elliptics_client->write_prepare(key, "", 0,total);
    async_result.wait();
    
    uint64_t offset = 0;

	int len = buflen;
	std::unique_ptr<char[]> post{ new char [len+1]};
	
	FCGX_Request *request = router->request;



	CryptoPP::SHA256 hash;

	while ( content_length > 0 )
	{

		len = ( len > content_length ? content_length : buflen );
		content_length -= len;

		FCGX_GetStr(post.get(), len, request->in);
		ioremap::elliptics::argument_data file( ioremap::elliptics::argument_data::pointer_type::from_raw(post.get(), len) );

		async_result = std::move(elliptics_client->write_plain(key, file, offset));
		async_result.wait();

		hash.Update( (byte*) post.get(), len);


		offset += len;
	}
	post.reset();

	byte digest[ CryptoPP::SHA256::DIGESTSIZE ];
	hash.Final(digest);

	CryptoPP::HexEncoder encoder;
	std::string sha256_hash ;
	encoder.Attach( new CryptoPP::StringSink( sha256_hash ) );

	encoder.Put( digest, sizeof(digest) );
	encoder.MessageEnd();
	
	async_result = elliptics_client->write_commit(key, "", offset, total);

	w.exec("INSERT INTO objects (id, name, is_dir, userid, tree, key, ctype, sha256, size) VALUES ("
			+ std::to_string(file_id) 
			+", " + w.quote(name) 
			+ ",  false, " 
			+ std::to_string(user->GetId()) + ", " 
			+ w.quote(new_tree) + ", "
			+ w.quote(key) + ", "
			+ w.quote(ctype) + ", "
			+ w.quote(sha256_hash) + ", "
			+ std::to_string(total) +")");
	
	w.commit();
	
	return key;
}

std::string File::GetProperty(std::string name) 
{
	return "";
}

void File::SetProperty(std::string name, std::string value) 
{
	
}

std::string Directory::GetProperty(std::string name) 
{
	return "";
}

void Directory::SetProperty(std::string name, std::string value) 
{
	return;
}

Directory::Directory(pqxx::connection *_db, User *_user)
{
	db = _db;
	user = _user;
	Id = user->GetDir();
	SetTree();
}

void Directory::SetTree()
{
	pqxx::work w(*db);
	r = new pqxx::result(w.exec("SELECT tree FROM objects where id=" + std::to_string(Id) ));
	auto row = r->begin();
	tree = row["tree"].as<std::string>();
	w.commit();
}

Directory* Directory::Create(std::string name)
{
	pqxx::work w(*db);
	r = new pqxx::result(w.exec("SELECT 1 from objects where tree ~ '" + tree + ".*{1}' and name="+w.quote(name) + " and userid=" + std::to_string(user->GetId())));
	if (!r->empty()) throw Error(Errors::OBJECT_EXIST);
	
	r = new pqxx::result(w.exec("SELECT nextval('objects_id_seq'::regclass)"));
	auto row = r->begin();
	unsigned int dir_id = row["nextval"].as<unsigned int>();
	std::string new_tree = tree + "." + std::to_string(dir_id);
	w.exec("INSERT INTO objects (id, name, is_dir, userid, tree) VALUES (" + std::to_string(dir_id) +", " + w.quote(name) + ",  true, " + std::to_string(user->GetId()) + ", " +w.quote(new_tree)+ ")");
	w.commit();
	
	Directory *result = new Directory(db, user);
	result->Chdir(dir_id);
	return result;
}

Directory* Directory::Create(unsigned int source_dirid, std::string name)
{
	Chdir(source_dirid);
	Create(name);
}

Json::Value Directory::Ls()
{
	pqxx::work w(*db);
	r = new pqxx::result(w.exec("SELECT id,name,is_dir from objects where tree ~ '" + tree + ".*{0,1}' and userid=" + std::to_string(user->GetId())));
	if (r->empty()) 
	{
		std::cerr << "tree="+tree+" userid="+std::to_string(user->GetId());
		throw Error(Errors::NOT_FOUND);
	}
	
	Json::Value result(Json::arrayValue);
	int i=0;
	//~ result[i]["id"] = Id;
	//~ result[i]["name"] = ".";
	//~ result[i]["is_dir"] = true;
	//~ ++i;
	for (auto row = r->begin(); row != r->end(); ++row)
	{
		result[i]["id"] = row["id"].as<unsigned int>();
		result[i]["name"] = row["name"].as<std::string>();
		result[i]["is_dir"] = row["is_dir"].as<bool>();
		++i;
	}
	return result;
}

Json::Value Directory::Ls(unsigned int source_dirid)
{
	Chdir(source_dirid);
	return Ls();
}

void Directory::Chdir(unsigned int _id)
{
	pqxx::work w(*db);
	r = new pqxx::result(w.exec("SELECT tree from objects where id=" + std::to_string(_id)));
	if (r->empty()) throw Error(Errors::NOT_FOUND);
	
	auto row = r->begin();
	tree = row["tree"].as<std::string>();
	Id = _id;
	w.commit();
}

std::string Directory::GetTree()
{
	return tree;
}


User::User(pqxx::connection *_db)
{
	db = _db;
}

User::~User()
{
}

std::string User::GetProperty(std::string name) 
{
	return "";
}

void User::SetProperty(std::string name, std::string value) 
{
	
}

void User::login(std::string _token)
{
	pqxx::work w(*db);
	token = _token;
	r = new pqxx::result(w.exec("SELECT userid FROM sessions where token=" + w.quote(token) +" and now() < expire" ));
	if (r->empty()) throw Error(Errors::TOKEN_INVALID);
	
	auto row = r->begin();
	Id = row["userid"].as<unsigned int>();
	
	r = new pqxx::result(w.exec("SELECT root_dir FROM users where id=" + std::to_string(Id) ));
	row = r->begin();
	root_dirId = row["root_dir"].as<unsigned int>();
	
	w.exec("UPDATE sessions set expire = now()+'1 day'::interval where (expire - '22 hours'::interval) < now() and token=" + w.quote(token) );
	w.commit();
	HttpStatus = Httpstatus::OK;
	ContentType = "text/plain; charset=utf-8";
}

std::string User::login(Json::Value &userdata)
{
	pqxx::work w(*db);
	std::string email = userdata["email"].asString();
	std::string password = userdata["password"].asString();
	std::string content = "";
	
	r = new pqxx::result(w.exec("SELECT id,password FROM users where email=" + w.quote(email) ));
	
	if (r->empty()) throw Error(Errors::AUTH_FALED);
	
	auto row = r->begin();
	
	std::string db_password = row["password"].as<std::string>();
	
	if (sha256(password) != db_password ) throw Error(Errors::AUTH_FALED);
	
	std::string userid = row["id"].as<std::string>();
	
	std::string token = "";
	do 
	{
		token = random_string(40);
		r = new pqxx::result(w.exec("SELECT 1 FROM sessions where token=" + w.quote(token) ));
	} while ( !r->empty() );
	
	w.exec("INSERT INTO sessions (userid,token) VALUES (" + userid + ", "+ w.quote(token) +")");
	
	w.commit();
	
	Json::StyledWriter sw;
	
	Json::Value root;
	root["token"] = token;
	content = sw.write(root);
	ContentType = "application/json; charset=utf-8";
	HttpStatus = Httpstatus::OK;
	
	return content;
}

bool User::add(Json::Value &userdata)
{
	pqxx::work w(*db);
	std::string email = userdata["email"].asString();
	std::string password = userdata["password"].asString();
	
	if (email == "") throw Error(Errors::EMAIL_NDEFINED);
	if (password == "") throw Error(Errors::PASS_NDEFINED);
	
	r = new pqxx::result(w.exec("SELECT id FROM users where email=" + w.quote(email) ));
   
    if (!r->empty()) throw Error(Errors::EMAIL_EXIST);
	
	r = new pqxx::result(w.exec("SELECT nextval('objects_id_seq'::regclass)"));
	auto row = r->begin();
	unsigned int dir_id = row["nextval"].as<unsigned int>();

	r = new pqxx::result(w.exec("SELECT nextval('users_id_seq'::regclass)"));
	row = r->begin();
	unsigned int user_id = row["nextval"].as<unsigned int>();

	
	w.exec("INSERT INTO users (id, email,password, root_dir) VALUES (" + std::to_string(user_id) +", " + w.quote(email) + ", "+ w.quote(sha256(password)) + ", " + std::to_string(dir_id) + ")");
	w.exec("INSERT INTO objects (id, name, is_dir, userid, tree) VALUES (" + std::to_string(dir_id) +", 'root',  true, " + std::to_string(user_id) + ", " +w.quote(dir_id)+ ")");
	
	w.commit();
	HttpStatus = Httpstatus::Created;
	
	return true;
}

std::string Base_Object::sha256(std::string str)
{
	CryptoPP::SHA256 hash;
	byte digest[CryptoPP::SHA256::DIGESTSIZE];
	std::string output;
	
	hash.CalculateDigest(digest,(const byte *)str.c_str(),str.size());
	
	CryptoPP::HexEncoder encoder;
	CryptoPP::StringSink *SS = new CryptoPP::StringSink(output);
	encoder.Attach(SS);
	encoder.Put(digest,sizeof(digest));
	encoder.MessageEnd();
	
	return output;
}

std::string Base_Object::random_string( size_t length )
{
    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;
}

unsigned int User::GetId()
{
	return Id;
}

unsigned int User::GetDir()
{
	return root_dirId;
}
