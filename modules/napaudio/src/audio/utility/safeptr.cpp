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
			enqueueAll();
			clear();
		}
		
		
		void DeletionQueue::registerSafeOwner(SafeOwnerBase* ptr)
		{
			mSafeOwnerList.emplace(ptr);
		}
		
		
		void DeletionQueue::unregisterSafeOwner(SafeOwnerBase* ptr)
		{
			mSafeOwnerList.erase(ptr);
		}
		
		
		void DeletionQueue::enqueueAll()
		{
			auto tempSafeOwnerList = mSafeOwnerList; // make a copy because enqueueForDeletion() will remove items from mSafeOwnerList.
			for (auto ptr : tempSafeOwnerList)
				ptr->enqueueForDeletion();
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

//            // Because we transfer ownership here we set the source's data pointer to nullptr.
//            source.mData = nullptr;
			
			auto newDeletionQueue = source.getDeletionQueue();
			newDeletionQueue->unregisterSafeOwner(&source); // unregister the previous source ptr
			source.setDeletionQueue(nullptr);  // empty the deletion queue value in the source ptr
			setDeletionQueue(newDeletionQueue);// set the deletion queue of this ptr
			newDeletionQueue->registerSafeOwner(this);       // register this ptr with the new deletion queue
		}
		
	}
	
}
