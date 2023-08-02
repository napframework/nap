/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "utility/stringutils.h"
#include "signalslot.h"
#include "rtti/object.h"

// External Includes
#include <memory>
#include <mutex>
#include <string>
#include <queue>
#include <thread>
#include <fstream>
#include <nap/datetime.h>
#include <atomic>

/**
 * This ugly macro allows us to register a nice, easy interface per log level
 * without making a long list of hard to read member functions to maintain.
 *
 * Logger::info("Hello %s number %.2f", "engine", 9);
 */

#define NAP_DECLARE_LOG_LEVEL(LEVEL, NAME)																						\
	static LogLevel& NAME##Level()																								\
	{																															\
		static LogLevel lvl(#NAME, LEVEL);																						\
		return lvl;																												\
	}																															\
																																\
	template<typename T>																										\
	static void NAME(T&& msg)																									\
	{																															\
		instance().log(LogMessage(NAME##Level(), std::forward<T>(msg)));														\
	}																															\
																																\
	static void	NAME(const rtti::Object& obj, const std::string& msg)															\
	{																															\
		instance().log(LogMessage(NAME##Level(), utility::stringFormat("%s: %s", obj.mID.c_str(), msg.c_str())));				\
	}																															\
																																\
	template <typename... Args>																									\
	static void NAME(const char* msg, Args&&... args)																			\
	{																															\
		instance().log(LogMessage(NAME##Level(), utility::stringFormat(msg, std::forward<Args>(args)...)));						\
	}																															\
																																\
	template <typename... Args>																									\
	static void NAME(rtti::Object& obj, const char* msg, Args&&... args)														\
	{																															\
		std::string msg_str = utility::stringFormat("%s: %s", obj.mID.c_str(), msg);											\
		instance().log(LogMessage(NAME##Level(), utility::stringFormat(msg_str.c_str(), std::forward<Args>(args)...)));			\
	}


namespace nap
{
	/**
	 * A LogLevel defines a tag for log messages associated to define what messages are output
	 * in a certain situation or configuration.
	 * Levels are ranked according to their level number.
	 * A higher number means it will be output in more situations.
	 * Examples are fatal error messages, warnings or debug messages.
	 */
	class NAPAPI LogLevel
	{
	public:
		/**
		 * Constructor taked the name of the level and a number to indicate where this level is ranked
		 * in comparison to other levels
		 */
		LogLevel(const std::string& name, int level) : mName(name), mLevel(level) {}
		LogLevel(const LogLevel&) = delete;
		LogLevel& operator=(const LogLevel&) = delete;

		// returns the level number to indicate the level's ranking
		int level() const								{ return mLevel; }
		const std::string& name() const					{ return mName; }

		// comparison operators
		bool operator<(const LogLevel& other) const		{ return this->mLevel < other.mLevel; }
		bool operator==(const LogLevel& other) const	{ return this->mLevel == other.mLevel; }
		bool operator<=(const LogLevel& other) const	{ return this->mLevel <= other.mLevel; }
		bool operator>=(const LogLevel& other) const	{ return this->mLevel >= other.mLevel; }

	private:
		std::string mName;
		int mLevel;
	};


	/**
	 * A LogMessage is tagged with a log level.
	 * Optionally it stores the object that outputs the message.
	 */
	class NAPAPI LogMessage
	{
	public:
		/**
		 * Log message constructor
		 */
		LogMessage(const LogLevel& lvl, const std::string& msg);

		/**
		 * Log message constructor, invoked when using temporary message (rvalue)
		 */
		LogMessage(const LogLevel& lvl , std::string&& msg);

		/**
		 * @return the log level of this message
		 */
		const LogLevel& level() const { return *mLevel; }

		/**
		 * @return the text associated with this message
		 */
		const std::string& text() const { return mMessage; }

		/**
		 * @return the message's timestamp in milliseconds since epoch
		 */
		const SystemTimeStamp& getTimestamp() const { return mTimeStamp; }

	private:
		const LogLevel* mLevel;
		const std::string mMessage;
		const SystemTimeStamp mTimeStamp;
	};

	// Shorthand for string formatter
	using LogMessageFormatter = std::function<std::string(const LogMessage& msg)>;

	/**
	 * Minimal log message formatter, containing only the level and the message
	 * @param msg the LogMessage to format
	 * @return string into which to write the formatted message
	 */
	std::string basicLogMessageFormatter(const LogMessage& msg);

	/**
	 * LogMessage string formatter that includes a timestamp in ISO format
	 * @param msg the LogMessage to format
	 * @return string into which to write the formatted message
	 */
	std::string timestampLogMessageFormatter(const LogMessage& msg);

	/**
	 * Abstract base class for log handlers.
	 */
	class NAPAPI LogHandler
	{
	public:
		LogHandler();
        virtual ~LogHandler() = default;

		/**
		 * Let this handler handle a log message. This call must be fast, don't block!
		 * WARNING: the implementer must handle thread safety.
		 * @param msg The log message to be handled
		 */
		virtual void commit(LogMessage msg) = 0;

		/**
		 * Set the log level on this handler, log messages lower than the provided level
		 * will not be sent to this handler.
		 * @param level The minimum level to be sent to this handler
		 */
		void setLogLevel(const LogLevel& level) { mLevel = &level; }

		/**
		 * @return The current log level of this handler, log messages lower than this level will
		 * not be handled by this logger.
		 */
		const LogLevel& getLogLevel() const { return *mLevel; }

		/**
		 * Override the basic log message formatter
		 * @param formatter the formatter to use when writing the log message if used by the derived class
		 */
		void setFormatter(LogMessageFormatter formatter);

		/**
		 * Format a log message using the currently set log formatter.
		 * @param msg the message to format
		 * @return a formatted log message
		 */
		std::string formatMessage(LogMessage& msg);

	private:
		const LogLevel* mLevel;
		LogMessageFormatter mFormatter = nullptr;
	};

	/**
	 * The logger is a singleton that can be called to log messages of various degrees of severity. 
	 * By default logged messages are printed to the console. 
	 * Invoke logToDirectory() to log messages to file.
	 */
	class NAPAPI Logger
	{
	public:
		/**
		 * Sets the current log level for all handlers.
		 * @param lvl new log level, messages lower than the selected log level won't be displayed.
		 */
		static void setLevel(const LogLevel& lvl)			{ instance().setCurrentLevel(lvl); }

		/**
		 * Sets the current log level for all handlers.
		 * @param level new log level, messages lower than the selected log level won't be displayed.
		 */
		void setCurrentLevel(const LogLevel& level);

		/**
		 * @return instance of the actual logger
		 */
		static Logger& instance();

		// this signal is emitted every time a log message is output.
		Signal<LogMessage> log;

		// all log messages will be displayed
		NAP_DECLARE_LOG_LEVEL(50, fine)

		// messages including debug messages
		NAP_DECLARE_LOG_LEVEL(100, debug)

		// general information about the proceedings of the app
		NAP_DECLARE_LOG_LEVEL(200, info)

		// warnings about unusual situations
		NAP_DECLARE_LOG_LEVEL(300, warn)

		// error messages indicate that something broke, but we're not ready to die yet
		NAP_DECLARE_LOG_LEVEL(400, error)

		// fatal errors that seriously hinder the functionality of the app
		NAP_DECLARE_LOG_LEVEL(500, fatal)

		/**
		 * @return all available log levels.
		 */
		static const std::vector<const LogLevel*>& getLevels()
		{
			// In-line so you will remember to amend this list
			// when you add or remove a LogLevel above.
			// Must be ordered by level value.
			static std::vector<const LogLevel*> lvls = {
					&fineLevel(),
					&debugLevel(),
					&infoLevel(),
					&warnLevel(),
					&errorLevel(),
					&fatalLevel()
			};
			return lvls;
		}

		/**
		 * @param name the name of the log level
		 * @return the corresponding log level or nullptr if it doesn't exist
		 */
		static const LogLevel* getLevel(const std::string& name);

		/**
		 * Convenience function to start logging to a file.
		 * @param filename The filename to write log entries to.
		 */
		static void addFileHandler(const std::string& filename);

		/**
		 * Start logging to a file in the specified directory.
		 * Writes all log information into a file with the current date/time in the name.
		 * Final log filename: {directory}/{prefix}_{timestamp}.log
		 * @param directory the directory to log to. 
		 * @param prefix name of the log_file.
		 */
		static void logToDirectory(const std::string& directory, const std::string& prefix = "log");

		/**
		 * Add a handler to this logger. The logger will take ownership of it.
		 * The handle will provide a log level that may filter out unwanted messages during logging.
		 * @param handler The handler to be invoked by this logger.
		 */
		void addHandler(std::unique_ptr<LogHandler> handler);

	private:
		// The logger is a singleton
		Logger();
		Logger(Logger const&);
		void operator=(Logger const&);

		void initialize();
		void onLog(const LogMessage& message);

		Slot<LogMessage> onLogSlot = {[&](LogMessage message)	{ onLog(message); }};
		const LogLevel* mLevel;
		std::vector<std::unique_ptr<LogHandler>> mHandlers{};
	};


	/**
	 * Log handler that will print to the console
	 */
	class ConsoleLogHandler : public LogHandler
	{
	public:
		ConsoleLogHandler() = default;

		/**
		 * Write the contents to the outputstream
		 * @param message
		 */
		void commit(LogMessage message) override;

	private:
		std::mutex mOutStreamMutex;
	};

	/**
	 * Log handler that will write log messages to a file.
	 * Upon construction, it will open a file stream to write to
	 * and will remain open for the lifetime of the handler.
	 * The file writer runs another thread to keep the call site unburdened
	 */
	class FileLogHandler : public LogHandler
	{
	public:
		FileLogHandler(const std::string& mFilename);
		~FileLogHandler();

		/**
		 * Write a message to the provided file
		 * @param message
		 */
		void commit(LogMessage message) override;

	private:
		void writeLoop();

		const std::string mFilename;				///< the filename we're writing to
		std::unique_ptr<std::thread> mWriteThread;	///< used to concurrently write to the file
		std::queue<LogMessage> mMessages;			///< commit will add to this queue and return
		std::mutex mQueueMutex;						///< to protect the queue
		std::atomic<bool> mRunning;					///< available for writing as long as we're running
	};


}
