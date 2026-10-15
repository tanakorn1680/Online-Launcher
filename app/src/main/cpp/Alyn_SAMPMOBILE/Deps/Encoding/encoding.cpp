#include "include/encoding.h"

#include <spdlog/spdlog.h>
#include <iconv.h>
#include <stdexcept>

static std::string g_charset = "CP1251"; // CP1251 | UTF-8 | CP874 | CP1252 | GBK

// True if s is well-formed UTF-8 that contains at least one multi-byte sequence.
static bool isValidUtf8WithMultibyte(const std::string& s)
{
	bool multibyte = false;
	size_t i = 0;
	const size_t n = s.size();
	while (i < n) {
		unsigned char c = (unsigned char) s[i];
		size_t len = 0;
		if (c < 0x80) {
			len = 1;
		}
		else if (c >= 0xC2 && c <= 0xDF) {
			len = 2;
		}
		else if (c >= 0xE0 && c <= 0xEF) {
			len = 3;
		}
		else if (c >= 0xF0 && c <= 0xF4) {
			len = 4;
		}
		else {
			return false;
		}

		if (i + len > n) {
			return false;
		}
		for (size_t k = 1; k < len; k++) {
			if (((unsigned char) s[i + k] & 0xC0) != 0x80) {
				return false;
			}
		}
		if (len > 1) {
			multibyte = true;
		}
		i += len;
	}
	return multibyte;
}

void Encoding::setCharset(const char* name)
{
	if (name != nullptr && name[0] != '\0') {
		g_charset = name;
		spdlog::info("Text encoding: {}", g_charset);
	}
}

std::string Encoding::cp2utf(const std::string& s)
{
	// Plain ASCII needs no conversion.
	bool ascii = true;
	for (unsigned char c : s) {
		if (c >= 0x80) {
			ascii = false;
			break;
		}
	}
	if (ascii) {
		return s;
	}

	if (g_charset == "UTF-8") {
		return s;
	}

	// A non-default code page may still receive UTF-8 from some servers.
	if (g_charset != "CP1251" && isValidUtf8WithMultibyte(s)) {
		return s;
	}

	iconv_t cd = iconv_open("UTF-8", g_charset.c_str());
	if (cd == (iconv_t) -1) {
		spdlog::error("iconv_open failed: {}", strerror(errno));
		return s;
	}

	std::string in = s;
	std::string out;
	out.resize(in.size() * 4);
	char* inbuf = const_cast<char*>(in.data());
	char* outbuf = const_cast<char*>(out.data());
	size_t inbytesleft = in.size();
	size_t outbytesleft = out.size();
	size_t res = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
	iconv_close(cd);
	if (res == (size_t) -1) {
		spdlog::error("iconv failed: {}", strerror(errno));
		return s;
	}
	out.resize(out.size() - outbytesleft);
	return out;
}
