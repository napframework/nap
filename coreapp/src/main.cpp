#include <jsonserializer.h>
#include <nap/core.h>
#include <thread>
#include <jsonrpcservice.h>
#include <rapidjson/error/en.h>
#include <rapidjson/filereadstream.h>

#ifndef APP_NAME
#define APP_NAME "unknown"
#endif


void testJSON() {
    using namespace rapidjson;
    using namespace nap;
    Document doc;

    std::string str = "{\n"
            "    \"name\": \"root\",\n"
            "    \"type\": \"nap::Entity\",\n"
            "    \"flags\": 7,\n"
            "    \"children\": [\n"
            "        {\n"
            "            \"name\": \"nap__RenderWindowComponent\",\n"
            "            \"type\": \"nap::RenderWindowComponent\",\n"
            "            \"flags\": 7,\n"
            "            \"attributes\": [\n"
            "                {\n"
            "                    \"name\": \"visible\",\n"
            "                    \"vType\": \"bool\",\n"
            "                    \"flags\": 7,\n"
            "                    \"value\": \"true\"\n"
            "                }\n"
            "            ]\n"
            "        },\n"
            "        {\n"
            "            \"name\": \"nap__PatchComponent\",\n"
            "            \"type\": \"nap::PatchComponent\",\n"
            "            \"flags\": 7,\n"
            "            \"children\": [\n"
            "                {\n"
            "                    \"name\": \"patch\",\n"
            "                    \"type\": \"nap::Patch\",\n"
            "                    \"flags\": 7\n"
            "                }\n"
            "            ]\n"
            "        }\n"
            "    ]\n"
            "}";


    doc.Parse(str);
    {
        FILE* fp = fopen("/home/bmod/Documents/nap/coreapp/resources/test.json", "r"); // non-Windows use "r"
        char readBuffer[65536];
        FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        doc.ParseStream(is);
    }
    if (doc.HasParseError()) {
        Logger::warn("JSON parse error: %s (%u)", GetParseError_En(doc.GetParseError()), doc.GetErrorOffset());
    }

    std::cout << doc.FindMember("name")->value.GetString() << std::endl;
//    std::cout << member

}

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

//	core.initialize();

    nap::JSONSerializer ser;
    ser.load("/home/bmod/Documents/nap/coreapp/resources/test.json", core);

    return 0;

    auto rpcService = core.getOrCreateService<nap::JsonRpcService>();
    rpcService->threaded.setValue(false);
	rpcService->running.setValue(true);


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
            nap::Logger::info("I'm alive");
		}
	}

	return 0;
}