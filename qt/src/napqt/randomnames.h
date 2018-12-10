#pragma once

#include <string>
#include <cstdlib>

namespace namegen
{
	int randint(int n);

	int randint(int min, int max);

	char randChar(const std::string& str);

	bool randBool();

	/**
	 * Random data generator
	 */
	class NameGen
	{
	public:
		NameGen();
		std::string multiple(int minLen = 1, int maxLen = 3);
		std::string single(int minLen = 2, int maxLen = 5);

	private:
		const std::string vowels = "aeiou";
		const std::string consonants = "bcdfghjklmnpqrstvwxyz";
	};

}