#pragma once

#include "stringutils.h"
#include <memory>
#include <mutex>
#include <nap/signalslot.h>
#include <string>

#define NAP_DECLARE_LOG_LEVEL(LEVEL, NAME)                                           \
	static const LogLevel& NAME##Level()                                             \
	{                                                                                \
		\
static LogLevel lvl(#NAME, LEVEL);                                                   \
		\
return lvl;                                                                          \
	\
}                                                                             \
	\
static void                                                                          \
	NAME(const std::string& msg)                                                     \
	{                                                                                \
		instance().log(LogMessage(NAME##Level(), msg));                              \
	}                                                                                \
	\
static void                                                                          \
	NAME(const Object& obj, const std::string& msg)                                  \
	{                                                                                \
		instance().log(LogMessage(NAME##Level(), msg, &obj));                        \
	}                                                                                \
	template <typename... Args>                                                      \
	static void NAME(const std::string& msg, Args... args)                           \
	{                                                                                \
		instance().log(LogMessage(NAME##Level(), stringFormat(msg, args...)));       \
	}                                                                                \
	template <typename... Args>                                                      \
	static void NAME(const Object& obj, const std::string& msg, Args... args)        \
	{                                                                                \
		instance().log(LogMessage(NAME##Level(), stringFormat(msg, args...), &obj)); \
	}



namespace nap
{
	class Object;

	class LogLevel
	{
	public:
		LogLevel(const std::string& name, int level) : mName(name), mLevel(level) {}
		int level() const { return mLevel; }
		const std::string& name() const { return mName; }

		bool operator<(const LogLevel& other) const { return this->mLevel < other.mLevel; }
		bool operator==(const LogLevel& other) const { return this->mLevel == other.mLevel; }
		bool operator<=(const LogLevel& other) const
		{
			return this->mLevel < other.mLevel || this->mLevel == other.mLevel;
		}

	private:
		std::string mName;
		int mLevel;
	};


	class LogMessage
	{
	public:
		LogMessage(const LogLevel& lvl, const std::string& msg, const Object* obj = nullptr)
			: mLevel(lvl), mMessage(msg), mObj(obj)
		{
		}

		const LogLevel& level() const { return mLevel; }
		const std::string& text() const { return mMessage; }
		const Object* object() const { return mObj; }

	private:
		const LogLevel mLevel;
		const std::string mMessage;
		const Object* mObj;
	};

	class Logger
	{
	public:
		static void setLevel(LogLevel lvl) { instance().mLevel = lvl; }
		static const LogLevel& getLevel() { return instance().mLevel; }

		// Singleton access
		static Logger& instance();

	public:
		Signal<LogMessage> log;

		NAP_DECLARE_LOG_LEVEL(50, fine)
		NAP_DECLARE_LOG_LEVEL(100, debug)
		NAP_DECLARE_LOG_LEVEL(200, info)
		NAP_DECLARE_LOG_LEVEL(300, warn)
		NAP_DECLARE_LOG_LEVEL(400, fatal)

	private:
		LogLevel mLevel;

		void onLog(LogMessage message);
		Slot<LogMessage> onLogSlot = {[&](LogMessage message) { onLog(message); }};

		Logger() : mLevel(debugLevel()) { log.connect(onLogSlot); }

		Logger(Logger const&) : mLevel(debugLevel()) {}
		void operator=(Logger const&);

		std::mutex outputMutex;
	};
}