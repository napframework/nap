/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "autoresetevent.h"

namespace nap
{
	namespace utility
	{
		void AutoResetEvent::reset()
		{
			// We first cancel any pending waits on this event.  If there are no pending waits, the cancellation will lead to the subsequent wait not being a blocking call
			cancelWait();

			// Wait for the signaled condition. Note that we don't need to explicitly reset anything here: wait already does that internally.
			// We can wait here because we did cancelWait() right before this.
			wait();
		}

		AutoResetEvent::EWaitResult AutoResetEvent::wait(const AutoResetEvent::LockedWaitCallback& callback)
		{
			// Wait for flag to be set
			std::unique_lock<std::mutex> lock(mMutex);
			mSignaledCondition.wait(lock, [this]() { return mSignaled || mWaitCanceled; });

			EWaitResult result = mSignaled ? EWaitResult::Completed : EWaitResult::Canceled;

			// Invoke the callback if there is one
			if (callback)
				callback(result);

			// Reset flags after being signaled
			mSignaled = false;
			mWaitCanceled = false;

			return result;
		}

		void AutoResetEvent::set(const AutoResetEvent::LockedSetCallback& callback)
		{
			{
				std::unique_lock<std::mutex> lock(mMutex);
				mSignaled = true;

				// Invoke the callback if there is one
				if (callback)
					callback();
			}

			mSignaledCondition.notify_all();
		}

		void AutoResetEvent::cancelWait()
		{
			{
				std::unique_lock<std::mutex> lock(mMutex);
				mWaitCanceled = true;
			}

			mSignaledCondition.notify_one();
		}
	}
}