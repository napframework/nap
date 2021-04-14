#include "controlthread.h"

namespace nap
{

	ControlThread::ControlThread(float rate)
	{
		setControlRate(rate);
	}


	ControlThread::~ControlThread()
	{
		if (mRunning)
			stop();
	}


	void ControlThread::setControlRate(float rate)
	{
		mWaitTime = MicroSeconds(static_cast<long>(1000000.0 / static_cast<double>(rate)));
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
		HighResolutionTimer timer;
		MicroSeconds frame_time;
		MicroSeconds delay_time;
		mRunning =  true;

		while (isRunning())
		{
			mTaskQueue.process();

			// Get time point for next frame
			frame_time = timer.getMicros() + mWaitTime;

			// update
			mUpdateSignal(std::chrono::duration_cast<Milliseconds>(delay_time).count());

			// Only sleep when there is at least 1 millisecond that needs to be compensated for
			// The actual outcome of the sleep call can vary greatly from system to system
			// And is more accurate with lower framerate limitations
			delay_time = frame_time - timer.getMicros();
			if(std::chrono::duration_cast<Milliseconds>(delay_time).count() > 0)
				std::this_thread::sleep_for(delay_time);
		}
	}


	ControlThread& ControlThread::get()
	{
		static ControlThread instance;
		return instance;
	}


}