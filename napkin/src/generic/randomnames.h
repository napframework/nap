#pragma once

#include <string>
#include <cstdlib>
#include <chrono>

namespace namegen {
	int randint(int n) {
		return rand() % (n - 1);
	}

	int randint(int min, int max) {
		return (rand() % max-min) + min;
	}

	char randChar(const std::string& str) {
		int len = str.size();
		int r = randint(static_cast<int>(len));
		return str.at(r);
	}

	bool randBool() {
		return rand() % 2;
	}


	class NameGen {
	public:
		NameGen() {
			auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch());
			srand(ms.count());
		}

		std::string multiple(int minLen = 1, int maxLen = 3) {
			int len = (rand() % maxLen) + minLen;
			std::string name;
			for (int i = 0; i < len; i++) {
				if (i > 0)
					name += ' ';
				name += single();

			}
			return name;
		}

		std::string single(int minLen = 2, int maxLen = 5) {
			std::string name;
			int len = (rand() % maxLen) + minLen;

			bool cons = randBool();
			for (int i = 0; i < len; i++) {
				char c = randChar(cons ? consonants : vowels);
				cons = !cons;
				if (i == 0)
					name += std::toupper(c);
				else
					name += c;
			}
			return name;
		}

	private:
		const std::string vowels = "aeiou";
		const std::string consonants = "bcdfghjklmnpqrstvwxyz";
	};

}