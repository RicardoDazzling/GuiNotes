#include "db.hpp"

std::vector<unsigned char> char_to_byte(const char* chars) {
	std::vector<unsigned char> bytes;
	for (int i = 0; i < sizeof(chars); i++) {
		bytes[i] = static_cast<unsigned char>(chars[i]);
	}
	return bytes;
}

void bytes_to_char(std::vector<unsigned char> bytes, char* chars) {
	unsigned int i = bytes.size();
	chars = new char[i];
	for (int x = 0; x < i; x++) {
		chars[x] = static_cast<char>(bytes[x]);
	}
}

std::vector<unsigned char> get_key(const char* password) {
	std::vector<unsigned char> key(32, 0x00);
	unsigned int length = sizeof(password);
	std::vector<unsigned char> pass_array = char_to_byte(password);
	for (int i = 0; i < 32; i++) {
		if (i < length) {
			key[i] = pass_array[i];
		}
		else {
			break;
		}
	}
	return pass_array;
}

void serialize(const char* password, char* hash) {
	// Hash the password
	SHA256 sha;
	std::string password_str(password);
	sha.update(password_str);
	std::array<uint8_t, 32> hash_array = sha.digest();
	std::string hash_str = SHA256::toString(hash_array);

	// Convert the hash string to a char array
	hash = new char[hash_str.length() + 1];
	hash[hash_str.length()] = '\0';
	for (int i = 0; i < hash_str.length(); i++) {
		hash[i] = hash_str[i];
	}
};

void encrypt(const char* password, const char* data, char* encrypted_data) {
	std::vector<unsigned char> key = get_key(password);
	AES aes(AESKeyLength::AES_256);
	std::vector<unsigned char> plain = char_to_byte(data);
	std::vector<unsigned char> encrypted = aes.EncryptCFB(plain, key, iv);
	bytes_to_char(encrypted, encrypted_data);
};

void decrypt(const char* password, const char* encrypted_data, char* data) {
	std::vector<unsigned char> key = get_key(password);
	AES aes(AESKeyLength::AES_256);
	std::vector<unsigned char> encrypted = char_to_byte(encrypted_data);
	std::vector<unsigned char> decrypted = aes.DecryptCFB(encrypted, key, iv);
	bytes_to_char(decrypted, data);
};

bool DB::Load(bool rise_exception = true) {
	std::ifstream file("db.xml");
	if (file.good()) {
		file.close();
	}
	else {
		std::ofstream file("db.xml");
		char hash;
		serialize("admin", &hash);

		file << "<database>\n"
			<< "	<user name=\"admin\" pass=\"" << hash << "\"\n"
			<< "</database>\n";
		file.close();
	}

	pugi::xml_parse_result result = this->doc.load_file("db.xml");
	if (!result) {
		if (rise_exception) {
			throw (std::string("ReadFileException: ") + std::string(result.description()));
		}
		return false;
	}
	return true;
}

DB::DB()
{
	this->Load();
};

DB::DB(const char* username, const char* password)
{
	this->Load();
	this->Login(username, password);
};

bool DB::Login(const char* username, const char* password, bool rise_exception = true) {
	if (this->logged_in) {
		this->Logout();
	}
	pugi::xml_node users = this->doc.child("database").child("user");
	auto user = users.find_child_by_attribute("name", username);
	char password_hash;
	serialize(password, &password_hash);
	if (user != PUGIXML_NULL) {
		if (user.attribute("pass").as_string() == std::string(1, password_hash)) {
			this->username = new char[sizeof(username)];
			this->password = new char[sizeof(password)];
			for (int i = 0; i < sizeof(username); i++) {
				this->username[i] = username[i];
			}
			for (int i = 0; i < sizeof(password); i++) {
				this->password[i] = password[i];
			}
			this->root = user;
		}
		else {
			if (rise_exception) {
				throw std::string("InvalidPasswordException: Password does not match");
			}
			return false;
		}
	}
	else {
		if (rise_exception) {
			throw std::string("InvalidUsernameException: Username does not exist");
		}
		return false;
	}
	this->logged_in = true;
	return true;
};

void DB::Logout() {
	this->logged_in = false;
	delete[] this->username;
	delete[] this->password;
	this->root.end();
};

char* DB::GetUsername() const {
	return this->username;
};

char* DB::GetPassword() const {
	return this->password;
};

bool DB::IsLoggedIn() const {
	return this->logged_in;
};

std::vector<std::tuple<std::string, std::string>> DB::GetNotes()
{
	if (!this->logged_in) {
		throw std::string("NotLoggedInException: User is not logged in");
	}
	pugi::xml_node notes = this->root.child("Note");
	std::vector<std::tuple<std::string, std::string>> note_list;
	for (pugi::xml_node note = notes.first_child(); note; note = note.next_sibling()) {
		std::string title = note.attribute("title").as_string();
		std::string content = note.attribute("content").as_string();
		note_list.push_back(std::tuple<std::string, std::string>(title, content));
	}
	return note_list;
};

void DB::SaveNoteTitle(std::string* old_title, std::string* new_title)
{
	if (!this->logged_in) {
		throw std::string("NotLoggedInException: User is not logged in");
	}
	pugi::xml_node notes = this->root.child("Note");
	auto note = notes.find_child_by_attribute("title", old_title->c_str());
	if (note != PUGIXML_NULL) {
		note.attribute("title").set_value(new_title->c_str());
	}
	else {
		throw std::string("NoteNotFoundException: Note with title " + *old_title + " does not exist");
	}
};
}
