#include <nap/core.h>
#include <jsonserializer.h>
#include <thread>

#ifndef APP_NAME
#define APP_NAME "unknown"
#endif

int main(int argc, char** argv)
{
    // Parse command line
    printf("NAP Skeleton\n");
    if (argc < 2) {
        printf("Usage:\n\t%s myfile.json [--keepalive]", APP_NAME);
        return 1;
    }

    bool keepAlive = false;
    if (argc > 2) {
        std::string arg(argv[2]);
        if (arg == "--keepalive")
            keepAlive = true;
    }

    // Setup core
    nap::Core core;
    core.initialize();

    { // Scope rapidjson
        nap::JSONDeserializer deser;
        std::string           filename(argv[1]);
        if (!deser.load(filename, core)) {
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