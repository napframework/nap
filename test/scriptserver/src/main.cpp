#include <nap.h>
#include <unistd.h>
#include <thread>

using namespace nap;


int main(int argc, char* argv[]) {
    Logger::setLevel(Logger::debugLevel());

    Core core;
    core.initialize();

    Entity &root = core.getRoot();
    RTTI::TypeInfo serverCompType = RTTI::TypeInfo::getByName("nap::PythonServerComponent");
    auto& server = root.addComponent(serverCompType);

    server.getAttribute<bool>("running")->setValue(true);

    while (true) {
        Logger::debug("Alive");
        sleep(10);
    }


    return 0;
}