#include <jsonserializer.h>
#include <nap/core.h>
#include <thread>
#include <jsonrpcservice.h>

#ifndef APP_NAME
#define APP_NAME "unknown"
#endif

int main(int argc, char** argv)
{
	// Parse command line
	printf("NAP Skeleton\n");

	bool keepAlive = true;
//	if (argc > 2) {
//		std::string arg(argv[2]);
//		if (arg == "--keepalive")
//			keepAlive = true;
//	}

	// Setup core
	nap::Core core;
	core.initialize();
	core.getOrCreateService<nap::JsonRpcService>()->running.setValue(true);

	if (argc == 2) {
		std::string filename(argv[1]);
		if (!nap::JSONSerializer().load(filename, core)) {
			nap::Logger::fatal("Failed to load data file, exiting.");
			return 1;
		}
	}

	if (keepAlive) {
		while (true) {
			std::this_thread::sleep_for(std::chrono::seconds(4));
		}
	}

	return 0;
}