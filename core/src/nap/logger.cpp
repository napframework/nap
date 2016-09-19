#include "logger.h"
#include <iostream>
#include "object.h"
#include "objectpath.h"

using namespace std;

namespace nap
{




	void Logger::onLog(LogMessage message)
	{
        if (message.level().level() < mLevel.level())
            return;

        outputMutex.lock();
        const Object* obj = message.object();
        if (obj) {
            cout << "LOG[" << message.level().name() << "] in " << ObjectPath(obj).toString() << ": " << message.text() << endl;
        } else {
            cout << "LOG[" << message.level().name() << "] " << message.text() << endl;
        }
        outputMutex.unlock();
	}


	Logger& Logger::instance()
	{
		static Logger instance;
		return instance;
	}
}