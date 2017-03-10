#include "logger.h"
#include "object.h"
#include "objectpath.h"

#include <iostream>

using namespace std;

namespace nap {
	void Logger::onLog(LogMessage message)
	{
        if (message.level().level() < mLevel.level())
            return;

        outputMutex.lock();
        const Object* obj = message.object();

        ostream* os = &cout;
        if (message.level().level() >= Logger::fatalLevel().level()) {
            os = &cerr;
        }

        if (obj) {
            *os << "LOG[" << message.level().name() << "] in " << ObjectPath(obj).toString() << ": " << message.text() << endl;
        } else {
            *os << "LOG[" << message.level().name() << "] " << message.text() << endl;
        }
        outputMutex.unlock();
	}


	Logger& Logger::instance()
	{
		static Logger instance;
		return instance;
	}
}
