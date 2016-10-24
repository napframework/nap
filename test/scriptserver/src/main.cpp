#include <nap.h>
#include <unistd.h>
#include <thread>

#include <mongoose/Server.h>
using namespace nap;


int main(int argc, char* argv[]) {
    Logger::setLevel(Logger::debugLevel());

    Core core;
    core.initialize();

    Entity &root = core.getRoot();
    auto& server = root.addComponent(RTTI::TypeInfo::getByName("nap::ScriptServerComponent"));

//    server.getAttribute<bool>("running")->setValue(true);

    Mongoose::Server srv(8888);
    srv.start();

    while (true) {
        Logger::debug("Alive");
        sleep(10);
    }


    return 0;
}