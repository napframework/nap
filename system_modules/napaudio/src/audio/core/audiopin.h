/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <set>
#include <mutex>

// RTTI includes
#include <rtti/rtti.h>

// Audio includes
#include <audio/utility/audiotypes.h>


namespace nap
{
	namespace audio
	{

		// Forward declarations
		class Node;
		class InputPin;
		class OutputPin;


		/**
		 * Interface for InputPin and MultiInputPin classes
		 */
		class NAPAPI InputPinBase
		{
			RTTI_ENABLE()

			friend class OutputPin;

		public:
			InputPinBase(Node* node);

			virtual ~InputPinBase();

			/**
			 * Connects a pin to this input. Disconnects the current connection first if necessary.
			 * For threadsafety connection is enqueued on the audio thread.
			 */
			void connect(OutputPin& input);
			
			/**
			 * Disconnects a pin from this input, if it is connected.
			 * For threadsafety connection is enqueued on the audio thread.
			 */
			void disconnect(OutputPin& input);
			
			/**
			 * Disconnects all pins connected to this pint.
			 * For threadsafety connection is enqueued on the audio thread.
			 */
			void disconnectAll();
						
			/**
			 * @return the node that owns this input.
			 */
			Node& getNode() { return *mNode; }

		private:
			/**
			 * Connects a pin to this input. Disconnects the current connection first if necessary.
			 */
			virtual void connectNow(OutputPin& input) = 0;

			/**
			 * Disconnects a pin from this input, if it is connected.
			 */
			virtual void disconnectNow(OutputPin& input) = 0;

			/**
			 * Disconnects all pins connected to this pint.
			 */
			virtual void disconnectAllNow() = 0;

			// The node that owns this input
			Node* mNode = nullptr;
		};


		/**
		 * An input pin is used by audio node to connect it to other nodes.
		 * The pin connects one channel (mono) audio.
		 */
		class NAPAPI InputPin final : public InputPinBase
		{
			RTTI_ENABLE(InputPinBase)

			friend class OutputPin;

		public:
			InputPin(Node* node) : InputPinBase(node) { }

			/**
			 * Destructor. If the input is connected on destruction the connection will be broken first.
			 */
			virtual ~InputPin() override;

			/**
			 * This method can be used by the node to pull one sample buffer output from the connected audio output.
			 * @return Pointer to the sample buffer containing the input coming in through the pin. If the InputPin is not connected or somewhere down the graph silence is being output nullptr can be returned.
			 */
			SampleBuffer* pull();
			
		private:
			/**
			 * Connects another node's OutputPin to this input.
			 * If either this input or the connected output is already connected it will be disconnected first.
			 * @param input: The output that this InputPin will be connected to.
			 */
			void connectNow(OutputPin& input) override;

			/**
			 * Disconnects this input from the specified output, if this connections exists.
			 */
			void disconnectNow(OutputPin& input) override;

			/**
			 * If connected, disconnects this pin.
			 */
			void disconnectAllNow() override;

			/**
			 * The audio output connected to this input.
			 * When it is a nullptr this input is not connected.
			 */
			OutputPin* mInput = nullptr;
		};


		/**
		 * An input pin is used by audio node to connect to other nodes.
		 * This pin can be connected to an arbitrary number of output pins belonging to different nodes.
		 * This is useful for example to build a mixing node that mixes any amount of input signals.
		 */
		class NAPAPI MultiInputPin final : public InputPinBase
		{
			RTTI_ENABLE(InputPinBase)

		public:
			MultiInputPin(Node* node, unsigned int reservedInputCount = 2);

			virtual ~MultiInputPin() override;

			/**
			 * This method can be used by the node to pull a buffer of samples for every connected output pin.
			 * A result verctor has to be passed as an argument.
			 * The vector will be resized and filled with a SampleBuffer* for each connected pin.
			 * The contents of the vector can be nullptr when somewhere down the conected graph an output returns nullptr.
			 * It is advised to allocate the result vector in the constructor of the Node class that contains the MultiInpuPin, and to prea	llocate memory using vector<>::reserve(), in order to avoid allocations on the audio thread.
			 * @param result vector that will be filled with pointers to a buffer for each connection.
			 */
			void pull(std::vector<SampleBuffer*>& result);


			/**
			 * Allocates memory to be able to handle the specified number of inputs without having to perform allocations on the audio thread.
			 * @param inputCount the maximum number of inputs that will be connected to this pin.
			 */
			void reserveInputs(unsigned int inputCount);

		private:
			/**
			 * Connect another node's output to this pin.
			 * @param input to be connected to this pin.
			 */
			void connectNow(OutputPin& input) override;

			/**
			 * Disconnect another node's output from this pin, if it is connected to this pin.
			 * @param input the pin to be disconnected from this pin
			 */
			void disconnectNow(OutputPin& input) override;

			/**
			 * Disconnects this input from all the connected pins.
			 */
			void disconnectAllNow() override;

			std::vector<OutputPin*> mInputs;
			std::vector<OutputPin*> mInputsCache;
		};


		/**
		 * An audio output is used by audio node to connect it to other nodes.
		 * The output connects one channel (mono) audio.
		 * It outputs a pointer to an owned SampleBuffer.
		 * The PullFunction of this class calls a calculate function on the node it belongs to.
		 */
		class NAPAPI OutputPin final
		{
			RTTI_ENABLE()

			friend class Node;
			friend class InputPinBase;
			friend class InputPin;
			friend class MultiInputPin;

		public:
			/**
			 * @param node the owner node if this output
			 */
			OutputPin(Node* node);

			~OutputPin();

			/**
			 * Disconnects the output from all connected inputs.
			 * For threadsafety connection is enqueued on the audio thread.
			 */
			void disconnectAll();

			/**
			 * Used by InputPin to poll this output for a new buffer of output samples
			 * @return nullptr if no output is available.
			 */
			SampleBuffer* pull();

			/**
			 * @return the node that owns this output.
			 */
			Node& getNode() { return *mNode; }

		protected:
			SampleBuffer mBuffer; ///< The buffer containing the latest output

		private:
			/**
			 * Disconnects the output from all connected inputs.
			 */
			void disconnectAllNow();

			// Used by the NodeManager to resize the internal buffers when necessary
			void setBufferSize(int bufferSize);

			// The node that owns this output
			Node* mNode = nullptr;

			// The inputs that this output is connected to
			// This list is kept so the connections can be broken on destruction.
			std::set<InputPinBase*> mOutputs;
		};

	}

}
