#include "testnetwork.h"

#include <nap.h>

#include <network.h>

using namespace nap;

bool testNetwork()
{
    nap::Core core;
    auto& entity = core.addEntity("entity");
    auto& httpClient = entity.addComponent<nap::HttpClient>();
	httpClient.host.setValue("www.engine9.nl");

	auto response = httpClient.get("depot/tommy/test.json");
	std::cout << response.errorMessage;
	std::cout << response.content;


    return true;
}