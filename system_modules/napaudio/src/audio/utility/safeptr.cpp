/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "safeptr.h"

namespace nap
{
	
	namespace audio
	{
		
		DeletionQueue::~DeletionQueue()
		{
			clear();
		}
		

		void DeletionQueue::clear()
		{
			std::unique_ptr<SafeOwnerBase::Data> toBeDeleted = nullptr;
			while (mQueue.try_dequeue(toBeDeleted))
			{
				toBeDeleted = nullptr;
			}
		}
		
		
		void SafeOwnerBase::assign(SafeOwnerBase& source)
		{
			// Make sure the object is not being assigned to itself.
			assert(&source != this);
			
			// When assigning from a different owner we first need to trash the current content (if any)
			enqueueForDeletion();
			
			// Then we copy the source data
			setData(std::move(source.getData()));

			auto newDeletionQueue = source.getDeletionQueue();
			source.setDeletionQueue(nullptr);  // empty the deletion queue value in the source ptr
			setDeletionQueue(newDeletionQueue);// set the deletion queue of this ptr
		}
		
	}
	
}
