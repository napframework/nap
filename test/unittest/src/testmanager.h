#pragma once

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <sstream>

typedef bool (*TestFunction)(void);

inline void padRight(std::string& str, const size_t num, const char paddingChar = ' ')
{
	if (num > str.size()) str.insert(str.end(), num - str.size(), paddingChar);
}

inline void padLeft(std::string& str, const size_t num, const char paddingChar = ' ')
{
	if (num > str.size()) str.insert(0, num - str.size(), paddingChar);
}

class Test
{
public:
	Test(const std::string& name, TestFunction function) : mName(name), mFunction(function) {}
	bool run() { return mFunction(); }
	const std::string& getName() { return mName; }

private:
	const std::string mName;
	const TestFunction mFunction;
	std::vector<std::string> errors;
};

class TestManager
{
public:
	TestManager() {}

	void runTests()
	{
		std::cout << "Running " << mTests.size() << " tests" << std::endl << std::endl;

		int padLen = findPadLength() + 3;

		std::vector<Test*> failedTests;

        int rowlength = 80;

		for (const auto& test : mTests) {
			std::string name = "__ " + test->getName() + " ";
			padRight(name, rowlength, '_');
			std::cout << name << std::endl;

			std::ostringstream result;
            result << " ";
			if (test->run()) {
				result << "OK";
			} else {
				result << "FAILED";
				failedTests.push_back(test.get());
			}
            result << " ==";
            std::string footer = result.str();
            padLeft(footer, rowlength, '=');

            std::cout << footer << std::endl << std::endl << std::endl;
		}

		std::cout << std::endl;

		if (failedTests.empty()) {
			std::cout << "All dandy!" << std::endl;
		} else {
			std::cout << "Tests failed: " << failedTests.size() << std::endl;
		}
	}

	void addTest(const std::string& name, TestFunction function)
	{
		mTests.push_back(std::make_unique<Test>(name, function));
	}

private:
	size_t findPadLength()
	{
		size_t len = 0;
		for (const auto& test : mTests)
			len = std::max<int>(test->getName().size(), len);
		return len;
	}


	std::vector<std::unique_ptr<Test>> mTests;
};


#define TEST_SETUP() TestManager testMan;

#define TEST_ADD(FUNC) testMan.addTest(#FUNC, &FUNC);

#define TEST_RUN() testMan.runTests();



#define TEST_ASSERT(CONDITION, REASON) \
	if (!(CONDITION)) {                \
		nap::Logger::fatal(REASON);    \
		return false;                  \
	}

#define TEST_ASSERT(CONDITION) TEST_ASSERT(CONDITION, "")
