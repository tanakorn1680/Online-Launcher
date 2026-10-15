#pragma once

#include <string>

class Encoding {
public:
	static std::string cp2utf(const std::string& s);

	// iconv name of the text encoding used by the server (CP1251 by default).
	static void setCharset(const char* name);
};
