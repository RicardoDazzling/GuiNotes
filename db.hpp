#ifndef GUI_DB
#define GUI_DB

#include "pugixml/pugixml.hpp"
#include "AES/AES.h"
#include "SHA256/SHA256.h"

#include <tuple>
#include <fstream>

std::vector<unsigned char> iv = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

std::vector<unsigned char> char_to_byte(const char* chars);
void bytes_to_char(std::vector<unsigned char> bytes, char* chars);
std::vector<unsigned char> get_key(const char* password);
void serialize(const char* password, char* hash);
void encrypt(const char* password, const char* data, char* encrypted_data);
void decrypt(const char* password, const char* encrypted_data, char* data);


class DB
{
private:
	char* username;
	char* password;
	bool logged_in = false;
	pugi::xml_document doc;
	//pugi::xml_parse_result result;
	pugi::xml_node root;
	bool Load(bool rise_exception = true);
public:
	DB();
	DB(const char* username, const char* password);
	bool Login(const char* username, const char* password, bool rise_exception = true);
	void Logout();
	char* GetUsername() const;
	char* GetPassword() const;
	bool IsLoggedIn() const;
	std::vector<std::tuple<std::string, std::string>> GetNotes();
	void SaveNoteTitle(std::string* old_title, std::string* new_title);
};


#endif // !GUI_DB
