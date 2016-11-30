#include "testmanager.h"

void TestManager::runTests()
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
