#include <iostream>
#include <nap/logger.h>

#include "testmanager.h"

// Tests
#include "tests/testcore.h"
#include "tests/testlink.h"
//#include "tests/testjson.h"
//#include "tests/testnetwork.h"
//#include "tests/testthreading.h"
//#include "tests/testscriptserver.h"

int main(int argc, char **argv) {
  nap::Logger::setLevel(nap::Logger::infoLevel());

  TEST_SETUP()

  TEST_ADD(testLink)
  TEST_ADD(testCore)
  TEST_ADD(testXMLSerializer)
  TEST_ADD(testObjectPath)
  //	TEST_ADD(testNetwork)
  //	TEST_ADD(testJson)
  //	TEST_ADD(testScriptServer)

  // TEST_ADD(testAsyncObserver)
  // TEST_ADD(testScheduler)

  TEST_RUN()

  char msg;
  std::cin >> msg;
  return 0;
}
