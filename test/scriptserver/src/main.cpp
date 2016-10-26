#include <nap.h>
#include <unistd.h>
#include <thread>

using namespace nap;


int main(int argc, char* argv[]) {
    Logger::setLevel(Logger::debugLevel());

    Core core;
    core.initialize();

    Entity &root = core.getRoot();

    for (int i=0; i < 1000; i++) {
        Entity& e = root.addEntity("TestEntity_" + std::to_string(i));

        for (int j=0; j < 3; j++) {
            e.addAttribute<float>("TestFloat" + std::to_string(j));
        }
    }


    RTTI::TypeInfo serverCompType = RTTI::TypeInfo::getByName("nap::JSONRPCServerComponent");
    auto& server = root.addComponent(serverCompType);

    server.getAttribute<bool>("running")->setValue(true);

    while (true) {
        Logger::debug("Alive");
        sleep(10);
    }


    return 0;
}