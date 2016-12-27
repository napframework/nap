#include <nap.h>
#include <nap/signalslot.h>
#include <stdlib.h>
#include <thread>
//#include <unistd.h>

using namespace nap;



void runScriptServerComponent()
{
	Logger::setLevel(Logger::debugLevel());
	Logger::info("Starting script server");

	Core core;
	core.initialize();

	Entity& root = core.getRoot();
	Entity& firstChild = root.addEntity("FirstChild");
	Entity& secondChild = root.addEntity("SecondChild");
    Entity& secondChildChild = secondChild.addEntity("SecondChildChild");
    root.addComponent<PatchComponent>();

	std::string serverTypename = "nap::JsonRpcService";
	RTTI::TypeInfo serverCompType = RTTI::TypeInfo::getByName(serverTypename);
	if (!serverCompType.isValid())
	{
		Logger::fatal("Type not found: %s", serverTypename.c_str());
		return;
	}
	auto server = core.getOrCreateService(serverCompType);

	server->getAttribute<bool>("running")->setValue(true);

    std::mutex mutex;


	while (true) {
//		std::string newName = "Root_" + std::to_string(rand() % 100);
////		Logger::debug("Changing name of root to %s", newName.c_str());
//        std::lock_guard<std::mutex> lock(mutex);
//        root.setName(newName);
		std::this_thread::sleep_for(std::chrono::seconds(4));
	}
}

//void testAsyncTCPServer()
//{
//
//	nap::AsyncTCPServer server(8888);
//
//	server.clientConnected.connect(
//		[&](AsyncTCPClient& client) { std::cout << "Connected: " << client.getIdent() << std::endl; });
//
//	server.clientDisconnected.connect(
//		[&](AsyncTCPClient& client) { std::cout << "Disconnected: " << client.getIdent() << std::endl; });
//
//	server.requestReceived.connect(
//		[&](AsyncTCPClient& client, const std::string& msg) { client.enqueueEvent("Replying to " + msg); });
//
//	while (true) {
//		std::this_thread::sleep_for(std::chrono::seconds(1));
//	}
//}

int main(int argc, char* argv[])
{

	runScriptServerComponent();
	return 0;
}