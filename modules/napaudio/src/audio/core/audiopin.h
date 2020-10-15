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
		public:
			InputPinBase(Node* node);
			
			virtual ~InputPinBase();
			
			/**
			 * Connects a pin to this input. Disconnects the current connection first if necessary.
			 */
			virtual void connect(OutputPin& input) = 0;
			
			/**
			 * Disconnects a pin from this input, if it is connected.
			 */
			virtual void disconnect(OutputPin& input) = 0;
			
			/**
			 * Disconnects all pins connected to this pint.
			 */
			virtual void disconnectAll() = 0;
			
			/**
			 * Returns wether this pin is connected to one or more other pins.
			 */
			virtual bool isConnected() const = 0;
			
			/**
			 * Enqueues a connect() call the be executed on the audio thread.
			 * This is the connect() function that is exposed to RTTR and to python.
			 */
			void enqueueConnect(OutputPin& pin);
			
			/**
			 * Enqueues a disconnect() call the be executed on the audio thread
			 * This is the disconnect() function that is exposed to RTTR and to python.
			 */
			void enqueueDisconnect(OutputPin& pin);
			
			/**
			 * @return the node that owns this input.
			 */
			Node& getNode() { return *mNode; }
		
		private:
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
			
			/**
			 * Connects another node's OutputPin to this input.
			 * If either this ipnut or the connected output is already connected it will be disconnected first.
			 * @param input: The output that this InputPin will be connected to.
			 */
			void connect(OutputPin& input) override;
			
			
			/**
			 * Disconnects this input from the specified output, if this connections exists.
			 */
			void disconnect(OutputPin& input) override;
			
			
			/**
			 * If connected, disconnects this pin.
			 */
			void disconnectAll() override;
			
			/**
			 * @return wether the input is connected
			 */
			bool isConnected() const override { return mInput != nullptr; }
		
		private:
			/*
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
			 * @return: the vector can contain nullptr items if somewhere down the line of connection silence is returned.
			 */
			std::vector<SampleBuffer*>& pull();
			
			/**
			 * Connect another node's output to this pin.
			 * @param input to be connected to this pin.
			 */
			void connect(OutputPin& input) override;
			
			/**
			 * Disconnect another node's output from this pin, if it is connected to this pin.
			 * @param input the pin to be disconnected from this pin
			 */
			void disconnect(OutputPin& input) override;
			
			/**
			 * Disconnects this input from all the connected pins.
			 */
			void disconnectAll() override;
			
			/**
			 * @return wether the input is connected to any outputs
			 */
			bool isConnected() const override { return !mInputs.empty(); }
			
			/**
			 * Allocates memory to be able to handle the specified number of inputs without having to perform allocations on the audio thread.
			 * @param inputCount the maximum number of inputs that will be connected to this pin.
			 */
			void reserveInputs(unsigned int inputCount);
		
		private:
			std::vector<OutputPin*> mInputs;
			std::vector<SampleBuffer*> mPullResult;
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
			 * @param parent: the owner node if this output
			 */
			OutputPin(Node* node);
			
			~OutputPin();
			
			/**
			 * Disconnects the output from all connected inputs.
			 */
			void disconnectAll();
			
			/**
			 * Checks wether the output is connected to any inputs
			 */
			bool isConnected() const
			{ return !mOutputs.empty(); }
			
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
