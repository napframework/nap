#include <nap/logger.h>

namespace nap
{
	std::string basicLogMessageFormatter(const LogMessage& msg)
	{
		return utility::stringFormat("[%s] %s", msg.level().name().c_str(), msg.text().c_str());
	}


	void ConsoleLogHandler::commit(LogMessage message)
	{
		bool isError = message.level() >= Logger::errorLevel();


		mOutStreamMutex.lock();

		if (isError)
			std::cerr << formatMessage(message) << std::endl;
		else
			std::cout << formatMessage(message) << std::endl;

		mOutStreamMutex.unlock();
	}
}
