#include "controlthread.h"

namespace nap
{

	ControlThread::ControlThread(float rate)
	{
		setDesiredControlRate(rate);
	}


	ControlThread::~ControlThread()
	{
		if (mRunning)
			stop();
	}


	void ControlThread::setDesiredControlRate(float rate)
	{
		mNewDesiredControlRate = rate;
	}


	void ControlThread::start()
	{
		if (!mRunning)
			mThread = std::make_unique<std::thread>([&](){ loop(); });
	}


	void ControlThread::stop()
	{
		mRunning = false;
		mThread->join();
		mThread = nullptr;
	}


	void ControlThread::enqueue(TaskQueue::Task task, bool waitUntilDone)
	{
		if (waitUntilDone)
		{
			std::condition_variable condition;
			std::mutex mutex;
			bool notified = false;
			auto taskPtr = &task;
			enqueue([&, taskPtr](){
			  std::unique_lock<std::mutex> lock(mutex);
			  (*taskPtr)();
			  notified = true;
			  condition.notify_one();
			});
			std::unique_lock<std::mutex> lock(mutex);
			condition.wait(lock, [&](){ return notified; });
			notified = false;
		}
		else
			mTaskQueue.enqueue(task);
	}


	void ControlThread::loop()
	{
		MicroSeconds frame_time;
		MicroSeconds delay_time;
		mRunning =  true;
		mLastTimeStamp = 0.f;
		HighResolutionTimer timer;
		timer.start();

		while (isRunning())
		{
			// Update desired control rate
			if (mNewDesiredControlRate != mDesiredControlRate)
			{
				mDesiredControlRate = mNewDesiredControlRate;
				mWaitTime = MicroSeconds(static_cast<long>(1000000.0 / static_cast<double>(mDesiredControlRate)));
			}

			// Get current time in milliseconds
			double new_elapsed_time = timer.getElapsedTime();

			// Calculate amount of milliseconds since last time stamp
			double delta_time = new_elapsed_time - mLastTimeStamp;

			// Store time stamp
			mLastTimeStamp = new_elapsed_time;

			// Update framerate
			calculateFramerate(delta_time);

			// Get time point for next frame
			frame_time = timer.getMicros() + mWaitTime;

			{
				std::unique_lock<std::mutex> mLock(mMutex);

				// Update
				mUpdateSignal(delta_time);

				// Process scheduled tasks
				mTaskQueue.process();
			}

			// Only sleep when there is at least 1 millisecond that needs to be compensated for
			// The actual outcome of the sleep call can vary greatly from system to system
			// And is more accurate with lower framerate limitations
			delay_time = frame_time - timer.getMicros();
			if (std::chrono::duration_cast<Milliseconds>(delay_time).count() > 0)
				std::this_thread::sleep_for(delay_time);
		}
	}


	ControlThread& ControlThread::get()
	{
		static ControlThread instance;
		return instance;
	}


	void ControlThread::calculateFramerate(double deltaTime)
	{
		mTicksum -= mTicks[mTickIdx];			// subtract value falling off
		mTicksum += deltaTime;					// add new value
		mTicks[mTickIdx] = deltaTime;			// save new value so it can be subtracted later */
		if (++mTickIdx == mTicks.size())		// inc buffer index
			mTickIdx = 0;
		mControlRate = static_cast<double>(mTicks.size()) / mTicksum;
	}


}