#pragma once

#include <string>
#include <cassert>
#include <limits>
#include <stdexcept>
#include <cctype>

class Base64
{
public:
	Base64();

	::std::string base64_encode(const ::std::string &bindata);
	::std::string base64_decode(const ::std::string &ascdata);

private:
	static const char b64_table[65];
	static const char reverse_table[128];
};

