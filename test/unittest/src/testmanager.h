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

    /**
     * Run all registered tests
     */
	void runTests();

    /**
     * Add a test, the provided function should return false if the test failed.
     * @param name The name of the test to be printed
     * @param function The function containing the test, should return false if the test failed
     */
	void addTest(const std::string& name, TestFunction function);

private:
	size_t findPadLength();
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

