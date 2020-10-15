#include "asyncobserver.h"

namespace nap
{
	namespace audio
	{
		
		void AsyncObserver::setBarrier(uint32_t aNumberOfNotifications)
		{
			mNumberOfNotifications = aNumberOfNotifications;
			if (mNumberOfNotifications == 0)
				mNotified = true;
		}
		
		
		void AsyncObserver::notifyBarrier()
		{
			std::unique_lock<std::mutex> lock(mMutex);
			if (++mNotificationCounter >= mNumberOfNotifications)
			{
				mNotified = true;
				mCondition.notify_one();
			}
		}
		
		
		void AsyncObserver::waitForNotifications()
		{
			if (!mNotified)
			{
				std::unique_lock<std::mutex> lock(mMutex);
				mCondition.wait(lock, [&]() { return mNotified; });
			}
			
			mNotificationCounter = 0;
			mNotified = false;
		}
		
		
		void AsyncObserver::notifyOne()
		{
			std::unique_lock<std::mutex> lock(mMutex);
			mNotified = true;
			mCondition.notify_one();
		}
		
		
		void AsyncObserver::notifyAll()
		{
			std::unique_lock<std::mutex> lock(mMutex);
			mNotified = true;
			mCondition.notify_all();
		}
		
	}
}