/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <utility/dllexport.h>

// Std includes
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
		class NAPAPI AsyncObserver
		{
		public:
			AsyncObserver() = default;
			~AsyncObserver() = default;
			
			/**
			 * Sets the number of performing threads that the waiting threads need to wait for to finish
			 * @param numberOfNotifications the number of notifications that will be collected until resuming after the wait.
			 */
			void setBarrier(uint32_t numberOfNotifications = 1);
			
			/**
			 * A performing thread needs to call this method to notify the waiting threads that it has finished.
			 */
			void notifyBarrier();
			
			/**
			 * The waiting threads call this method to wait for all performing threads to finish.
			 */
			void waitForNotifications();
			
			/**
			 * In the case of a single waiting thread, a performing thread can call this method to wake up the waiting thread and continue its execution.
			 */
			void notifyOne();
			
			/**
			 * In case of multiple waiting threads, a performing thread can call this method to wake up the waiting threads and continue their execution.
			 */
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
