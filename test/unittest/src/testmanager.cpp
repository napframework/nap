#include <nap/logger.h>
#include "testmanager.h"

void TestManager::runTests()
{
    nap::Logger::info("Running %d tests\n", mTests.size());

	int padLen = findPadLength() + 3;

	std::vector<Test*> failedTests;

	int rowlength = 80;

	for (const auto& test : mTests) {
		std::string name = "__ " + test->getName() + " ";
		padRight(name, rowlength, '_');
        nap::Logger::info(name);

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

        nap::Logger::info(footer + "\n\n");
	}


	if (failedTests.empty()) {
        nap::Logger::info("All dandy!");
	} else {
        nap::Logger::info("Tests failed: %d", failedTests.size());
	}
}


void TestManager::addTest(const std::string& name, TestFunction function)
{
	mTests.push_back(std::make_unique<Test>(name, function));
}


size_t TestManager::findPadLength()
{
	size_t len = 0;
	for (const auto& test : mTests)
		len = (size_t) std::max<int>((const int&) test->getName().size(), len);
	return len;
}
