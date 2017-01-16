#include <nap/core.h>
#include <thread>
//#include <jsonrpcservice.h>
//#include <rapidjson/error/en.h>
//#include <rapidjson/filereadstream.h>
//#include <rapidjson/document.h>

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

//    testJSON();
//    return 0;

	core.initialize();

//    nap::JSONSerializer ser;
//    ser.load("/home/bmod/Documents/nap/coreapp/resources/test.json", core);
//    printf(ser.toString(core.getRoot()).c_str());

//    return 0;

    std::string rpcServiceTypename = "nap::JsonRpcService";
    RTTI::TypeInfo rpcServiceType = RTTI::TypeInfo::getByName(rpcServiceTypename);
    if (!rpcServiceType.isValid()) {
        nap::Logger::fatal("Failed to retrieve type: '%s'", rpcServiceTypename.c_str());
        return -1;
    }
    auto rpcService = core.getOrCreateService(rpcServiceType);
    rpcService->getAttribute<bool>("threaded")->setValue(false);
    rpcService->getAttribute<bool>("running")->setValue(true);


//	if (argc == 2) {
//		std::string filename(argv[1]);
//		if (!nap::JSONSerializer().load(filename, core)) {
//			nap::Logger::fatal("Failed to load data file, exiting.");
//			return 1;
//		}
//	}

	if (keepAlive) {
		while (true) {
			std::this_thread::sleep_for(std::chrono::seconds(4));
            nap::Logger::info("I'm alive");
		}
	}

	return 0;
}