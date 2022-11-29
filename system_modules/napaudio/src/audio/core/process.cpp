/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "process.h"

#include <audio/core/audionode.h>
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
		
		Process::Process(ParentProcess& parent) : mNodeManager(&parent.getNodeManager())
		{
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
			auto childPtr = &child;
			getNodeManager().enqueueTask([&, childPtr]() {
				auto it = std::find(mChildren.begin(), mChildren.end(), childPtr);
				if (it == mChildren.end())
					mChildren.emplace_back(childPtr);
			});
		}
		
		
		void ParentProcess::removeChild(Process& child)
		{
			auto childPtr = &child;
			getNodeManager().enqueueTask([&, childPtr]() {
				auto it = std::find(mChildren.begin(), mChildren.end(), childPtr);
				if (it != mChildren.end())
					mChildren.erase(it);
			});
		}
		
		
		void ParentProcess::processSequential()
		{
			for (auto& child : mChildren)
			{
				if (child != nullptr)
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
						if (child != nullptr)
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
