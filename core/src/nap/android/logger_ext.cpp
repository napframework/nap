/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <nap/logger.h>
#include <android/log.h>

// The logging tag message will appear under on Android
static const char* ANDROID_LOG_TAG = "NAPFramework";

namespace nap
{
	std::string basicLogMessageFormatter(const LogMessage& msg)
	{
		// Use Android's own log levels, don't prepend to message text
		return utility::stringFormat("%s", msg.text().c_str());
	}


	void ConsoleLogHandler::commit(LogMessage message)
	{
		bool isError = message.level() >= Logger::errorLevel();

		mOutStreamMutex.lock();

		int priority = -1;
		std::string logLevelName = message.level().name();
		if (logLevelName == "fine")
		{
			priority = ANDROID_LOG_VERBOSE;
		} else if (logLevelName == "debug")
		{
			priority = ANDROID_LOG_DEBUG;
		} else if (logLevelName == "info")
		{
			priority = ANDROID_LOG_INFO;
		} else if (logLevelName == "warn")
		{
			priority = ANDROID_LOG_WARN;
		} else if (logLevelName == "error")
		{
			priority = ANDROID_LOG_ERROR;
		} else if (logLevelName == "fatal")
		{
			priority = ANDROID_LOG_FATAL;
		}
		
		if (priority != -1)
		{
			__android_log_print(priority, ANDROID_LOG_TAG, "%s", formatMessage(message).c_str());		
		} else 
		{
			Logger::warn("Unknown log level: %s", logLevelName.c_str());
			Logger::info(message.text());
		}

		mOutStreamMutex.unlock();
	}
}
