#include <iostream>
#include <nap/logger.h>

#include "testmanager.h"

// Tests
#include "tests/testcore.h"
#include "tests/testlink.h"

int main(int argc, char** argv)
{
	nap::Logger::setLevel(nap::Logger::infoLevel());

	TEST_SETUP()

//	TEST_ADD(testXMLSerializer)
	TEST_ADD(testLink)
	TEST_ADD(testCore)
	TEST_ADD(testObjectPath)
    TEST_ADD(testJSONSerializer)

	TEST_RUN()
}
