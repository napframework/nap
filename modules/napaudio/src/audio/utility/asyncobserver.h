#pragma once

#include <thread>
#include <condition_variable>

namespace nap
{
	namespace audio
	{
		
		/**
		 * The AsyncObserver will assist you when one or more threads need to wait for any number of other threads to finish performing certain tasks.
		 * We will call these threads respectively the waiting threads and the performing threads.
		 */
		class AsyncObserver
		{
		public:
			AsyncObserver();
			~AsyncObserver() = default;
			
			/**
			 * Sets the number of performing threads that the waiting threads need to wait for to finish
			 * @param numberOfNotifications the number of notifications that will be collected until resuming after the wait.
			 */
			void setBarrier(uint32_t numberOfNotifications = 1);
			
			/**
			 *
			 */
			void notifyBarrier();
			
			void waitForNotifications();
			
			void notifyOne();
			
			void notifyAll();
			
		private:
			std::condition_variable mCondition;
			std::mutex mMutex;
			
			bool mNotified = false;
			uint32_t mNumberOfNotifications = 0;
			uint32_t mNotificationCounter = 0;
		};
		
	}
}
