#include <nap/signalslot.h>

#include <nap/logger.h>

namespace nap
{
    
    // This function is a helper to call the logger from cpp in order to avoid a circular dependency between signalslot.h and logger.h
    void logInfo(const std::string& message)
    {
        Logger::info(message);
    }
    
}
