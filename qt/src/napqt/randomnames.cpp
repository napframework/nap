/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "randomnames.h"
#include <chrono>
#include <random>
#include <cctype>

int namegen::randint(int n)
{
	return rand() % (n - 1);
}

int namegen::randint(int min, int max)
{
	std::random_device seeder;
	std::mt19937 engine(seeder());
	std::uniform_int_distribution<int> dist(min, max);
	return dist(engine);
}

char namegen::randChar(const std::string& str)
{
	int len = str.size();
	int r = randint(static_cast<int>(len));
	return str.at(r);
}

bool namegen::randBool()
{
	return rand() % 2 == 0;
}

namegen::NameGen::NameGen()
{
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch());
	srand(ms.count());
}

std::string namegen::NameGen::multiple(int minLen, int maxLen)
{
	int len = (rand() % maxLen) + minLen;
	std::string name;
	for (int i = 0; i < len; i++)
	{
		if (i > 0)
			name += ' ';
		name += single();

	}
	return name;
}

std::string namegen::NameGen::single(int minLen, int maxLen)
{
	std::string name;
	int len = (rand() % maxLen) + minLen;

	bool cons = randBool();
	for (int i = 0; i < len; i++)
	{
		char c = randChar(cons ? consonants : vowels);
		cons = !cons;
		if (i == 0)
			name += std::toupper(c);
		else
			name += c;
	}
	return name;
}
