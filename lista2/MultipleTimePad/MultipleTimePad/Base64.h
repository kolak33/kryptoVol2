#pragma once
#include <stdio.h>
#include <string.h>

class Base64
{
public:
	Base64();
	
	int Decode(char* b64message, unsigned char** buffer, size_t* length);
	int Encode(const unsigned char* buffer, size_t length, char** b64text);

private:
	size_t CalcDecodeLength(const char* b64input);
};

