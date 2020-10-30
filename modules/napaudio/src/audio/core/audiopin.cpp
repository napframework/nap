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
	RTTI_FUNCTION("connect", &nap::audio::InputPinBase::enqueueConnect)
	RTTI_FUNCTION("disconnect", &nap::audio::InputPinBase::enqueueDisconnect)
	RTTI_FUNCTION("isConnected", &nap::audio::InputPinBase::isConnected)
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
		
		
		void InputPinBase::enqueueConnect(OutputPin& pin)
		{
			OutputPin* pinPtr = &pin;
			pin.mNode->getNodeManager().enqueueTask([&, pinPtr]() {
				connect(*pinPtr);
			});
		}
		
		
		void InputPinBase::enqueueDisconnect(OutputPin& pin)
		{
			OutputPin* pinPtr = &pin;
			pin.mNode->getNodeManager().enqueueTask([&, pinPtr]() {
				disconnect(*pinPtr);
			});
		}
		
		// --- InputPin --- //
		
		InputPin::~InputPin()
		{
			disconnectAll();
		}
		
		
		SampleBuffer* InputPin::pull()
		{
			if (mInput)
				return mInput->pull();
			else
				return nullptr;
		}
		
		
		void InputPin::connect(OutputPin& input)
		{
			// remove old connection
			if (mInput)
				mInput->mOutputs.erase(this);
			
			// make the input and output point to one another
			mInput = &input;
			mInput->mOutputs.emplace(this);
		}
		
		
		void InputPin::disconnect(OutputPin& input)
		{
			if (&input == mInput)
			{
				mInput->mOutputs.erase(this);
				mInput = nullptr;
			}
		}
		
		
		void InputPin::disconnectAll()
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
			mPullResult.reserve(reservedInputCount);
			mInputsCache.reserve(reservedInputCount);
		}
		
		
		MultiInputPin::~MultiInputPin()
		{
			disconnectAll();
		}
		
		
		std::vector<SampleBuffer*>& MultiInputPin::pull()
		{
			// Make a copy of mInputs because its contents can change while pulling its content.
			mInputsCache = mInputs;
			
			mPullResult.clear();
			for (auto& input : mInputsCache)
				mPullResult.emplace_back(input->pull());
			
			return mPullResult;
		}
		
		
		void MultiInputPin::connect(OutputPin& input)
		{
			auto it = std::find(mInputs.begin(), mInputs.end(), &input);
			if (it == mInputs.end())
				mInputs.emplace_back(&input);
			input.mOutputs.emplace(this);
		}
		
		
		void MultiInputPin::disconnect(OutputPin& input)
		{
			auto it = std::find(mInputs.begin(), mInputs.end(), &input);
			if (it != mInputs.end())
				mInputs.erase(it);
			input.mOutputs.erase(this);
		}
		
		
		void MultiInputPin::disconnectAll()
		{
			while (!mInputs.empty())
			{
				auto input = *mInputs.begin();
				input->mOutputs.erase(this);
				auto it = std::find(mInputs.begin(), mInputs.end(), input);
				assert(it != mInputs.end());
				mInputs.erase(it);
			}
			mPullResult.clear();
			mInputsCache.clear();
		}
		
		
		void MultiInputPin::reserveInputs(unsigned int inputCount)
		{
			mPullResult.shrink_to_fit();
			mInputsCache.shrink_to_fit();
			mPullResult.reserve(inputCount);
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
			disconnectAll();
		}
		
		
		void OutputPin::disconnectAll()
		{
			while (!mOutputs.empty())
				(*mOutputs.begin())->disconnect(*this);
		}
		
		
		SampleBuffer* OutputPin::pull()
		{
			mNode->update();
			return &mBuffer;
		}
		
		
		void OutputPin::setBufferSize(int bufferSize)
		{
			mBuffer.resize(bufferSize, 0.f);
		}
		
	}
}
