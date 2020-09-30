/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <string>
#include <cstdlib>

/**
 * Random generation for testing purposes
 */
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