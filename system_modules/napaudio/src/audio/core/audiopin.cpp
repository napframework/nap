/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "audiopin.h"

// Audio includes
#include <audio/core/audionode.h>
#include <audio/core/audionodemanager.h>

#include <nap/logger.h>

// Std includes
#include <cassert>

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


		void InputPinBase::connect(OutputPin& pinToConnect)
		{
			auto outputPin = &pinToConnect;
			auto inputPin = this;
			
			// Get the safeptrs of the nodes to pass to the lambda, so these ptrs can be checked before trying to make a connection (preventing calling connectNow() with Nodes that are (being) removed).
			auto inputNode = getNode().getSafe();
			auto outputNode = outputPin->getNode().getSafe();

			getNode().getNodeManager().enqueueTask([inputPin, outputPin, inputNode, outputNode](){
				if (inputNode != nullptr && outputNode != nullptr)
				{
					auto inputNodeRaw = rtti_cast<Node>(inputNode.get());
					auto outputNodeRaw = rtti_cast<Node>(outputNode.get());
					assert(inputNodeRaw != nullptr);
					assert(outputNodeRaw != nullptr);
					if (inputNodeRaw->mInputs.find(inputPin) == inputNodeRaw->mInputs.end())
					{

						Logger::warn("InputPinBase::connect(): Input pin not found for Node with type: %s", inputNodeRaw->get_type().get_name().to_string().c_str());
					}
					else if (outputNodeRaw->mOutputs.find(outputPin) == outputNodeRaw->mOutputs.end())
					{
						Logger::warn("InputPinBase::connect(): Output pin not found for Node with type: %s", outputNodeRaw->get_type().get_name().to_string().c_str());
					}
					else
						inputPin->connectNow(*outputPin);
				}
			});
		}


		void InputPinBase::disconnect(OutputPin& pinToDisconnect)
		{
			auto outputPin = &pinToDisconnect;
			auto inputPin = this;
			auto inputNode = getNode().getSafe();
			auto outputNode = outputPin->getNode().getSafe();

			getNode().getNodeManager().enqueueTask([inputPin, outputPin, inputNode, outputNode]() {
				if (inputNode != nullptr && outputNode != nullptr)
				{
					auto inputNodeRaw = rtti_cast<Node>(inputNode.get());
					auto outputNodeRaw = rtti_cast<Node>(outputNode.get());
					assert(inputNodeRaw != nullptr);
					assert(outputNodeRaw != nullptr);
					if (inputNodeRaw->mInputs.find(inputPin) == inputNodeRaw->mInputs.end())
					{

						Logger::warn("InputPinBase::disconnect(): Input pin not found for Node with type: %s", inputNodeRaw->get_type().get_name().to_string().c_str());
					}
					else if (outputNodeRaw->mOutputs.find(outputPin) == outputNodeRaw->mOutputs.end())
					{
						Logger::warn("InputPinBase::disconnect(): Output pin not found for Node with type: %s", outputNodeRaw->get_type().get_name().to_string().c_str());
					}
					else
						inputPin->disconnectNow(*outputPin);
				}
			});
		}


		void InputPinBase::disconnectAll()
		{
			auto node = getNode().getSafe();
			auto inputPin = this;

			getNode().getNodeManager().enqueueTask([inputPin, node]() {
				if (node != nullptr)
				{
					auto nodeRaw = rtti_cast<Node>(node.get());
					assert(nodeRaw != nullptr);
					if (nodeRaw->mInputs.find(inputPin) == nodeRaw->mInputs.end())
					{
						Logger::warn("InputPinBase::disconnectAll(): Input pin not found for Node with type: %s", nodeRaw->get_type().get_name().to_string().c_str());

					}
					else
						inputPin->disconnectAllNow();
				}
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
