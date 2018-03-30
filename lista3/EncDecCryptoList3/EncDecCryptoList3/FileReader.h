#pragma once

#include <vector>

class FileReader
{
public:
	FileReader();
	~FileReader();

	std::vector<char> ReadAllBytes(char const* filename);
	char* ReadToArray(char const* filename, unsigned int &len);
};

