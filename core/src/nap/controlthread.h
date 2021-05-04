#pragma once

#include <utility/threading.h>
#include <nap/core.h>

namespace nap
{

	/**
	 * A control thread that can be used to perform control of audio outside the main thread.
	 * Can also be used as singleton. This is necessary when running multiple nap cores, because libpython only allows one python interpreter per process.
	 * The thread runs a loop on a specified speed.
	 */
	class NAPAPI ControlThread
	{
	public:
		/**
		 * Constructor
		 * @param rate initial control rate
		 */
		ControlThread(float rate = 60.f);
		~ControlThread();

		/**
		 * Set the desired control rate. This is the maximum speed at which the control loop is capped.
		 * @param rate in Hz
		 */
		void setDesiredControlRate(float rate);

		/**
		 * @return The desired control rate. This is the maximum speed at which the control loop is capped.
		 */
		float getDesiredControlRate() const { return mDesiredControlRate; }

		/**
		 * Starts the thread, if it is not running already.
		 */
		void start();

		/**
		 * Stops the thread, waits for the current cycle to finish before returning.
		 */
		void stop();

		/**
		 * @return wether the thread is currently running
		 */
		bool isRunning() const { return mRunning; }

		/**
		 * Connect a slot or function to a signal that is triggered every cycle of the loop.
		 * @param slot Can be anything that can be passed to Signal::connect()
		 */
		template <typename T>
		void connectPeriodicTask(T& slot);

		/**
		 * Disconnect a slot or function to a signal that is triggered every cycle of the loop.
		 * @param slot Can be anything that can be passed to Signal::connect()
		 */
		template <typename T>
		void disconnectPeriodicTask(T& slot);

		/**
		 * Enqueue a task to be executed once on the next cycle.
		 * @param task
		 * @param waitUntilDone if true, the method will wait for returning until the task is performed.
		 */
		void enqueue(TaskQueue::Task task, bool waitUntilDone = false);

		/**
		 * @return global instance of the control thread.
		 */
		static ControlThread& get();

		/**
		 * @return number of frames per second
		 */
		float getControlRate() const { return mControlRate; }

		/**
		 * Use this mutex to acquire a lock on the processing done within the contol thead's loop.
		 * If a lock is acquired using this mutex, you can be sure that there is no interference with any of the connected periodic tasks.
		 * @return a mutex object that can be used to lock the control thread using a std::unique_lock object.
		 */
		std::mutex& getMutex() { return mMutex; }

	private:
		void loop();

		// Calculates the framerate over time
		void calculateFramerate(double deltaTime);

		// The control rate cap
		float mDesiredControlRate = 0.0f;
		std::atomic<float> mNewDesiredControlRate = { 0.0f };

		// Current actual framerate
		std::atomic<float> mControlRate = { 0.0f };

		// Used to calculate framerate over time
		std::array<double, 20> mTicks;
		double mTicksum = 0;
		uint32 mTickIdx = 0;
		double mLastTimeStamp = 0;
		MicroSeconds mWaitTime;

		std::atomic<bool> mRunning = { false };
		TaskQueue mTaskQueue;
		std::unique_ptr<std::thread> mThread = nullptr;
		Signal<double> mUpdateSignal;
		std::mutex mMutex;
	};


	template <typename T>
	void ControlThread::connectPeriodicTask(T& slot)
	{
		if (!mRunning.load())
			mUpdateSignal.connect(slot);
		else {
			auto slotPtr = &slot;
			enqueue([&, slotPtr](){ mUpdateSignal.connect(*slotPtr); });
		}
	}


	template <typename T>
	void ControlThread::disconnectPeriodicTask(T& slot)
	{
		if (!mRunning.load())
			mUpdateSignal.disconnect(slot);
		else {
			auto slotPtr = &slot;
			enqueue([&, slotPtr](){ mUpdateSignal.disconnect(*slotPtr); }, true);
		}
	}


}