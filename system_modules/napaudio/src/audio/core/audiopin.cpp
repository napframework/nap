/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "audiopin.h"

// Std includes
#include <cassert>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/core/audionodemanager.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::InputPinBase)
	RTTI_FUNCTION("connect", &nap::audio::InputPinBase::connect)
	RTTI_FUNCTION("disconnect", &nap::audio::InputPinBase::disconnect)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::InputPin)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::MultiInputPin)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::OutputPin)
RTTI_END_CLASS

namespace nap
{
	namespace audio
	{
		
		// --- InputPinBase --- //
		
		
		InputPinBase::InputPinBase(Node* node) : mNode(node)
		{
			mNode->mInputs.emplace(this);
		}
		
		
		InputPinBase::~InputPinBase()
		{
			mNode->mInputs.erase(this);
		}


		void InputPinBase::connect(OutputPin& connection)
		{
			auto connectionPtr = &connection;

			getNode().getNodeManager().enqueueTask([&, connectionPtr](){
				connectNow(*connectionPtr);
			});
		}


		void InputPinBase::disconnect(OutputPin& connection)
		{
			auto connectionPtr = &connection;

			getNode().getNodeManager().enqueueTask([&, connectionPtr]() {
				disconnectNow(*connectionPtr);
			});
		}


		void InputPinBase::disconnectAll()
		{
			getNode().getNodeManager().enqueueTask([&]() {
				disconnectAllNow();
			});
		}


		// --- InputPin --- //
		
		InputPin::~InputPin()
		{
			disconnectAllNow();
		}
		
		
		SampleBuffer* InputPin::pull()
		{
			if (mInput)
				return mInput->pull();
			else
				return nullptr;
		}
		
		
		void InputPin::connectNow(OutputPin& connection)
		{
			// remove old connection
			if (mInput)
				mInput->mOutputs.erase(this);

			// make the input and output point to one another
			mInput = &connection;
			mInput->mOutputs.emplace(this);
		}
		
		
		void InputPin::disconnectNow(OutputPin& connection)
		{
			if (&connection == mInput)
			{
				mInput->mOutputs.erase(this);
				mInput = nullptr;
			}
		}
		
		
		void InputPin::disconnectAllNow()
		{
			if (mInput)
			{
				mInput->mOutputs.erase(this);
				mInput = nullptr;
			}
		}
		
		
		// --- MultiInputPin ---- //
		
		MultiInputPin::MultiInputPin(Node* node, unsigned int reservedInputCount) : InputPinBase(node)
		{
			mInputsCache.reserve(reservedInputCount);
		}
		
		
		MultiInputPin::~MultiInputPin()
		{
			disconnectAllNow();
		}
		
		
		void MultiInputPin::pull(std::vector<SampleBuffer*>& result)
		{
			// Make a copy of mInputs because its contents can change while pulling its content.
			mInputsCache = mInputs;
			
			result.resize(mInputsCache.size());
			for (auto i = 0; i < mInputsCache.size(); ++i)
				result[i] = mInputsCache[i]->pull();
		}
		
		
		void MultiInputPin::connectNow(OutputPin& connection)
		{
			auto it = std::find(mInputs.begin(), mInputs.end(), &connection);
			if (it == mInputs.end())
				mInputs.emplace_back(&connection);
			connection.mOutputs.emplace(this);
		}
		
		
		void MultiInputPin::disconnectNow(OutputPin& connection)
		{
			auto it = std::find(mInputs.begin(), mInputs.end(), &connection);
			if (it != mInputs.end())
				mInputs.erase(it);
			connection.mOutputs.erase(this);
		}


		void MultiInputPin::disconnectAllNow()
		{
			while (!mInputs.empty())
			{
				auto input = *mInputs.begin();
				input->mOutputs.erase(this);
				auto it = std::find(mInputs.begin(), mInputs.end(), input);
				assert(it != mInputs.end());
				mInputs.erase(it);
			}
			mInputsCache.clear();
		}
		
		
		void MultiInputPin::reserveInputs(unsigned int inputCount)
		{
			mInputsCache.shrink_to_fit();
			mInputsCache.reserve(inputCount);
		}
		
		
		
		// --- OutputPin --- //
		
		
		OutputPin::OutputPin(Node* node)
		{
			node->mOutputs.emplace(this);
			mNode = node;
			setBufferSize(mNode->getBufferSize());
		}
		
		
		OutputPin::~OutputPin()
		{
			mNode->mOutputs.erase(this);
			disconnectAllNow();
		}
		
		
		void OutputPin::disconnectAll()
		{
			getNode().getNodeManager().enqueueTask([&]() {
				disconnectAllNow();
			});
		}
		
		
		SampleBuffer* OutputPin::pull()
		{
			mNode->update();
			return &mBuffer;
		}


		void OutputPin::disconnectAllNow()
		{
			while (!mOutputs.empty())
				(*mOutputs.begin())->disconnectNow(*this);
		}


		void OutputPin::setBufferSize(int bufferSize)
		{
			mBuffer.resize(bufferSize, 0.f);
		}
		
	}

}
