/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <mutex>
#include <functional>
#include <condition_variable>

namespace nap
{
	namespace utility
	{
		/** 
		 * Helper class that implements an AutoResetEvent on top of condition_variable.
		 * You can use this to wait for an event to happen; once the wait completes, the event is reset and you can wait on the event again.
		 * If the event is set before the wait is entered, the wait completes immediately and resets the event.
		 */
		class AutoResetEvent
		{
		public:
			/**
			 * The result of the wait on the event
			 */
			enum class EWaitResult
			{
				Completed,			///< The wait completed successfully (i.e. the event was signaled normally)
				Canceled			///< The wait was canceled
			};

			using LockedWaitCallback = std::function<void(EWaitResult)>;
			using LockedSetCallback = std::function<void()>;

			/**
			 * Resets the event to a non-signaled, non-canceled state
			 */
			void reset();

			/**
			 * Wait for the event to be signaled. This is an infinite wait, which can be canceled by calling cancelWait()
			 * @param callback An optional callback that is called when the wait completes. This callback has the special property that it is executed during the same lock as the set() function. 
			 *				   This makes the code executing in the callback effectively an 'atomic' operation with the wait.
			 * @return The result of the wait (completed or canceled)
			 */
			EWaitResult wait(const LockedWaitCallback& callback = LockedWaitCallback());

			/**
			 * Signal the event, unblocking any threads waiting on it
			 * @param callback An optional callback that is called when the set completes. This callback has the special property that it is executed during the same lock as the wait() function.
			 *				   This makes the code executing in the callback effectively an 'atomic' operation with the set.
			 */
			void set(const LockedSetCallback& callback = LockedSetCallback());

			/**
			 * If any threads are waiting on the event, cancel their waits
			 */
			void cancelWait();

		private:
			std::mutex				mMutex;						///< Mutex used to guard the event state
			std::condition_variable mSignaledCondition;			///< Condition used to wait on
			bool					mSignaled = false;			///< Whether the event has been signaled
			bool					mWaitCanceled = false;		///< Whether the wait has been canceled
		};
	}
}
