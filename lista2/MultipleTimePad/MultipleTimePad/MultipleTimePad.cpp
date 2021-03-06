// MultipleTimePad.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include "Base64.h"
#include <random>
#include <set>
#include <cctype>



void CreateTestCiphers(std::vector<std::vector<unsigned char>> &ciphertexts)
{
	std::vector<std::string> messages = 
	{
		"wiadomosc pierwsza the ziomek",
		"no a to jest druga wiad elopr",
		"what a beautiful day to creat",
		"An awful looking code with co",
		"complexity of bogo sort shit.",
		"Constructing encryption schem",
		"From a pseudorandom generator",
		"are you gonna stand and fight",
		"suddenly i will play my ace .",
		"see right throuh you any hour"
	};

	std::vector<unsigned char> key;
	for (int i = 0; i < messages[0].size(); ++i)
		key.push_back((unsigned char)(rand() % 256));

	ciphertexts.clear();
	ciphertexts.resize(messages.size());
	for (int i = 0; i < messages.size(); ++i)
	{
		for (int j = 0; j < messages[i].size(); ++j)
			ciphertexts[i].push_back((unsigned char)messages[i][j] ^ key[j]);
	}
}

void ProcessFile(std::string strFile, int iNumCiphers, std::vector<std::vector<unsigned char>> &ciphertexts)
{
	std::ifstream file(strFile.c_str());
	if (!file.good()) return;

	std::istream_iterator<std::string> iter(file);
	ciphertexts.clear();
	ciphertexts.resize(iNumCiphers);
	int iCurrCipher = -1;

	for (; iter != std::istream_iterator<std::string>(); ++iter)
	{
		if (*iter == "ciphertext")
		{
			++iCurrCipher;
			++iter; //omit 'ciphertext'
			++iter; //omit '#Num'
		}
		ciphertexts[iCurrCipher].push_back((unsigned char)std::stoi(*iter, nullptr, 2));
		unsigned char value = 0;
		for (int i = 0; i < iter->size(); ++i)
		{
			value <<= 1;
			if ((*iter)[i] == '1')
				value += 1;
		}
		if (ciphertexts[iCurrCipher].back() != value) std::cout << "BLAD CIPHER" << std::endl;
	}

	std::swap(ciphertexts[0], ciphertexts[ciphertexts.size() - 1]); // i want to guess last message
	file.close();
}

void XORCiphertexts(std::vector<std::vector<unsigned char>> &ciphertexts, std::vector<std::vector<unsigned char>> &xoredCiphers)
{
	xoredCiphers.resize(ciphertexts.size() - 1);
	for (unsigned int ciphertextsIter = 1; ciphertextsIter < ciphertexts.size(); ++ciphertextsIter)
	{
		for (unsigned int firstCipherIter = 0; firstCipherIter < ciphertexts[0].size(); ++firstCipherIter)
		{
			if (firstCipherIter < ciphertexts[ciphertextsIter].size())
				xoredCiphers[ciphertextsIter - 1].push_back((unsigned char)(ciphertexts[0][firstCipherIter] ^ ciphertexts[ciphertextsIter][firstCipherIter]));
		}
	}
}

void CreateAlphabet(std::vector<unsigned char> &alphabet)
{
	alphabet.clear();

	for (int i = 65; i <= 90; ++i) // accepted ASCII
		alphabet.push_back((unsigned char)i);

	for (int i = 97; i <= 122; ++i) // accepted ASCII
		alphabet.push_back((unsigned char)i);

	std::vector<unsigned char> acceptedValues = { ' ', '.', ',', '\'', ':', ';', '!', '-', '?' };

	for (unsigned char symbol : acceptedValues)
		alphabet.push_back(symbol);
}

bool ValidateAlpha(unsigned char alpha, unsigned char xoredChar)
{
	unsigned char value = (unsigned char)(alpha ^ xoredChar);

	static std::vector<unsigned char> acceptedValues = { ' ', '.', ',', '\'', ':', ';', '!', '-', '?' };
	//if ((value >= 32 && value <= 126)) // akceptowalny kod ASCII
	if (value >= 97 && value <= 122)
	{
		return true;
	}
	else if (value >= 65 && value <= 90 || value == 0)
	{
		return true;
	}
	else if (std::any_of(acceptedValues.begin(), acceptedValues.end(), [value](unsigned char letter) {return letter == value; }))
		return true;
	else
		return false;
}

bool ValidateCiphers(unsigned char alpha, unsigned int firstCipherPosIter, std::vector<std::vector<unsigned char>> &xoredCiphers)
{
	int validatedCount = 0;
	int maxPossibleValidation = 0;
	for (unsigned int ciphertextsIter = 0; ciphertextsIter < xoredCiphers.size(); ++ciphertextsIter)
	{
		if (firstCipherPosIter < xoredCiphers[ciphertextsIter].size())
		{		
			++maxPossibleValidation;
			if (ValidateAlpha(alpha, xoredCiphers[ciphertextsIter][firstCipherPosIter]))
			{		
				++validatedCount;
			}
		}
		else
		{
			continue;
		}
	}
	maxPossibleValidation = std::max(1, maxPossibleValidation); // in case maxPossibleValidation is zero
	float successRatio = (float)validatedCount / (float)maxPossibleValidation;
	return (successRatio >= 1.0);
}

void GuessMessage(std::vector<std::vector<unsigned char>> &xoredCiphers)
{
	std::vector<unsigned char> alphabet;
	CreateAlphabet(alphabet);

	std::vector<std::vector<unsigned char>> possibleSolutions;
	possibleSolutions.resize(xoredCiphers[0].size());
	for (unsigned int firstCipherIter = 0; firstCipherIter < xoredCiphers[0].size(); ++firstCipherIter)
	{
		for (unsigned char alpha : alphabet)
		{
			if (ValidateCiphers(alpha, firstCipherIter, xoredCiphers))
				possibleSolutions[firstCipherIter].push_back(alpha);
		}
	}

	for (int i = 0; i < possibleSolutions.size(); ++i)
	{
		std::cout << i << ":";
		for (int j = 0; j < possibleSolutions[i].size(); ++j)
		{
			std::cout << possibleSolutions[i][j] << "|";
		}
		std::cout << std::endl;
	}
}

void GuessMessageDistinctCharacters(std::vector<std::vector<unsigned char>> &xoredCiphers)
{
	std::vector<unsigned char> alphabet;
	CreateAlphabet(alphabet);

	std::vector<std::set<unsigned char>> possibleSolutions;
	possibleSolutions.resize(xoredCiphers[0].size());
	for (unsigned int firstCipherIter = 0; firstCipherIter < xoredCiphers[0].size(); ++firstCipherIter)
	{
		for (unsigned char alpha : alphabet)
		{
			if (ValidateCiphers(alpha, firstCipherIter, xoredCiphers))
				possibleSolutions[firstCipherIter].insert(std::tolower(alpha));
		}
	}

	for (int i = 0; i < possibleSolutions.size(); ++i)
	{
		std::cout << i << ":";
		for (auto iter = possibleSolutions[i].begin(); iter != possibleSolutions[i].end(); ++iter)
		{
			std::cout << *iter << "|";
		}
		std::cout << std::endl;
	}
}

bool GuessWord(std::vector<std::vector<unsigned char>> &xoredCiphers, std::string word)
{
	bool wordGuessedCorrectly = false;
	std::vector<std::vector<std::string>> possibleSolutions;
	possibleSolutions.resize(xoredCiphers[0].size());

	std::vector<std::vector<std::string>> possibleSecondSolutions;
	possibleSecondSolutions.resize(xoredCiphers[0].size());
	bool endGuess = false;
	for (unsigned int firstCipherIter = 0; firstCipherIter < xoredCiphers[0].size() && !endGuess; ++firstCipherIter)
	{
		int validated = 0;
		int maxPossibleValidation = word.size(); // not true at the end of message but whatever	
		for (unsigned char letter : word)
		{
			if (firstCipherIter < xoredCiphers[0].size())
			{
				if (ValidateCiphers(letter, firstCipherIter, xoredCiphers))
					++validated;
				++firstCipherIter; // check next position
			}
			else
			{
				endGuess = true;
				break;
			}
		}
		firstCipherIter -= word.size(); 

		float ratio = (float)validated / (float)maxPossibleValidation;
		if (ratio >= 0.85)
		{
			std::string hiddenMsg;
			unsigned int iter = firstCipherIter;
			for (unsigned char letter : word)
				hiddenMsg.push_back((unsigned char)(letter ^ xoredCiphers[0][iter++]));
			possibleSecondSolutions[firstCipherIter].push_back(hiddenMsg);
			possibleSolutions[firstCipherIter].push_back(word);
			wordGuessedCorrectly = true;

			std::cout << "WORDS FOR: " << word << std::endl;
			for (int j = 0; j < xoredCiphers.size(); ++j)
			{
				std::string hiddenMsg;
				unsigned int iter = firstCipherIter;
				for (unsigned char letter : word)
					hiddenMsg.push_back((unsigned char)(letter ^ xoredCiphers[j][iter++]));
				std::cout << hiddenMsg << "\n";
			}
		}
	}

	for (int i = 0; i < possibleSecondSolutions.size(); ++i)
		for (int j = 0; j < possibleSecondSolutions[i].size(); ++j)
			if (possibleSecondSolutions[i][j] != "")
				std::cout << "pos: " << i << ":" << possibleSecondSolutions[i][j] << " |normal word = " << word << std::endl;

	return wordGuessedCorrectly;
}

void CheckWords(std::vector<std::vector<unsigned char>> &xoredCiphers)
{
	std::ifstream file("engDictionary.txt");
	if (!file.good())
		return;

	std::istream_iterator<std::string> iter(file);

	for (int i = 0; iter != std::istream_iterator<std::string>(); iter++, i++)
	{
		if (i % 10000 == 0) std::cout << "zgaduje slowo nr: " << i << std::endl;
		if (iter->size() >= 4)
			GuessWord(xoredCiphers, *iter);
	}

	file.close();
}

void CheckBible(std::vector<std::vector<unsigned char>> &xoredCiphers)
{
	std::ifstream file("romeoAndJuliet.txt");
	if (!file.good())
		return;

	std::istream_iterator<std::string> iter(file);
	std::set<std::string> wordsSet;

	for (int i = 0; iter != std::istream_iterator<std::string>(); iter++, i++)
		wordsSet.insert(*iter);

	int i = 0;
	for (auto iter = wordsSet.begin(); iter != wordsSet.end(); iter++, i++)
	{
		if (i % 10000 == 0) std::cout << "zgaduje slowo nr: " << i << std::endl;
		if (iter->size() >= 3)
			GuessWord(xoredCiphers, *iter);
	}

	file.close();
}

int main()
{
	std::vector<std::vector<unsigned char>> ciphertexts;
	int iNumCiphertexts = 20;
	//CreateTestCiphers(ciphertexts);
	ProcessFile("ciphertexts.txt", iNumCiphertexts, ciphertexts);
	std::vector<std::vector<unsigned char>> xoredCiphers;
	XORCiphertexts(ciphertexts, xoredCiphers);

	//CheckBible(xoredCiphers);
	//CheckWords(xoredCiphers);
	std::string word = "MERCUTIO god ye good den, fair gentlewoman";
	GuessWord(xoredCiphers, word);
	//GuessMessage(xoredCiphers);
	GuessMessageDistinctCharacters(xoredCiphers);

    return 0;
}

