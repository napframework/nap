/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "process.h"

#include <audio/core/audionodemanager.h>
#include <audio/core/audiopin.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::Process)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::ParentProcess)
RTTI_END_CLASS

namespace nap
{
	namespace audio
	{
		// --- Process  ---//

		Process::Process(NodeManager& nodeManager) : mNodeManager(&nodeManager)
		{
		}


		Process::Process(ParentProcess& parent) : mNodeManager(&parent.getNodeManager())
		{
		}


		Process::~Process()
		{
			// Unregister as root process, if needed
			auto it = std::find_if(getNodeManager().mRootProcesses.begin(), getNodeManager().mRootProcesses.end(), [&](auto& e){ return e.get() == this; });
			if (it != getNodeManager().mRootProcesses.end())
				getNodeManager().mRootProcesses.erase(it);

			// Unregister as process
			if (mRegisteredWithNodeManager.load())
				getNodeManager().unregisterProcess(*this);
		}


		void Process::update()
		{
			if (mLastCalculatedSample < getSampleTime())
			{
				mLastCalculatedSample = getSampleTime();
				process();
			}
		}
		
		
		int Process::getBufferSize() const
		{
			return getNodeManager().getInternalBufferSize();
		}
		
		
		float Process::getSampleRate() const
		{
			return getNodeManager().getSampleRate();
		}
		
		
		DiscreteTimeValue Process::getSampleTime() const
		{
			return getNodeManager().getSampleTime();
		}
		
		
		// --- ParentProcess --- //
		
		
		void ParentProcess::addChild(Process& child)
		{
			auto childPtr = child.getSafe();
			auto parentPtr = getSafe();
			getNodeManager().enqueueTask([&, parentPtr, childPtr]() {
				// Check if parent and child are still valid
				if (parentPtr == nullptr || childPtr == nullptr)
					return;

				// Check for duplicates
				auto it = std::find_if(mChildren.begin(), mChildren.end(), [&](auto& e){ return e.get() == childPtr.get(); });
				if (it == mChildren.end())
					mChildren.emplace_back(childPtr);
			});
		}
		
		
		void ParentProcess::removeChild(Process& child)
		{
			auto childPtr = child.getSafe();
			auto parentPtr = getSafe();
			getNodeManager().enqueueTask([&, parentPtr, childPtr]() {
				// Check if parent and child are still valid
				if (parentPtr == nullptr || childPtr == nullptr)
					return;

				auto it = std::find_if(mChildren.begin(), mChildren.end(), [&](auto& e){ return e.get() == childPtr.get(); });
				if (it != mChildren.end())
					mChildren.erase(it);
			});
		}
		
		
		void ParentProcess::processSequential()
		{
			for (auto& child : mChildren)
			{
				if (child != nullptr) // Check if the child is not enqueued for deletion in the meantime
					child->update();
			}
		}
		
		
		void ParentProcess::processParallel()
		{
			auto parallelCount = std::min<int>(mThreadPool.getThreadCount(), mChildren.size());
			mAsyncObserver.setBarrier(parallelCount);
			for (auto threadIndex = 0; threadIndex < parallelCount; ++threadIndex)
			{
				mThreadPool.execute([&, threadIndex]() {
					auto i = threadIndex;
					while (i < mChildren.size()) {
						auto& child = mChildren[i];
						if (child != nullptr) // Check if the child is not enqueued for deletion in the meantime
							child->update();
						i += mThreadPool.getThreadCount();
					}
					mAsyncObserver.notifyBarrier();
				});
			}
			mAsyncObserver.waitForNotifications();
		}
		
		
		void ParentProcess::process()
		{
			// First remove children that are (being) deleted or enqueued for deletion
			auto it = mChildren.begin();
			while (it != mChildren.end())
			{
				if ((*it) == nullptr)
					it = mChildren.erase(it);
				else
					it++;
			}

			switch (mMode)
			{
				case Mode::Sequential:
					processSequential();
					break;
				case Mode::Parallel:
					processParallel();
					break;
			}
		}
		
		
	}
	
}
