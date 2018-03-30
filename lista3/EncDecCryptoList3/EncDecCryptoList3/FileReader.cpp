#include "stdafx.h"
#include "FileReader.h"

#include <fstream>

FileReader::FileReader()
{
}


FileReader::~FileReader()
{
}

std::vector<char> FileReader::ReadAllBytes(char const* filename)
{
	std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
	std::ifstream::pos_type pos = ifs.tellg();

	std::vector<char> result(pos);

	ifs.seekg(0, std::ios::beg);
	ifs.read(result.data(), pos);

	return result;
}

char* FileReader::ReadToArray(char const* filename, unsigned int &len)
{
	std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
	std::ifstream::pos_type pos = ifs.tellg();

	char *text = new char[pos];

	ifs.seekg(0, std::ios::beg);
	ifs.read(text, pos);

	len = pos;

	return text;
}