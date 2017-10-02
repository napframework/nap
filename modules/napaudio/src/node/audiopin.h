#pragma once

// Std includes
#include <set>
#include <mutex>

// Audio includes
#include <utility/audiotypes.h>


namespace nap {
    
    namespace audio {
        
        // Forward declarations
        class Node;
        class InputPin;
        class OutputPin;
        
        
        class NAPAPI InputPinBase
        {
        public:
            virtual void connect(OutputPin& input) = 0;
            virtual void disconnect(OutputPin& input) = 0;
            virtual void disconnectAll() = 0;
            virtual bool isConnected() const = 0;
        };
        
        
        /**
         * An audio input is used by audio node to connect it to other nodes.
         * The input connects one channel (mono) audio.
         */
        class NAPAPI InputPin : public InputPinBase
        {
            friend class OutputPin;
            
        public:
            InputPin() = default;
            
            /**
             * Destructor. If the input is connected on destruction the connection will be broken first.
             */
            virtual ~InputPin();
            
            /**
             * This method can be used by the node to pull one sample buffer output from the connected audio output.
             * @return If the InputPin is not connected or somewhere down the graph silence is being output nullptr can be returned.
             */
            SampleBufferPtr pull();
            
            /**
             * Connects another node's @OutputPin to this input.
             * If either this ipnut or the connected output is already connected it will be disconnected first.
             * @param input: The output that this @InputPin will be connected to.
             */
            void connect(OutputPin& input) override;
            
            
            /**
             * Disconnects this input from the specified output
             */
            void disconnect(OutputPin& input) override;
            
            
            /**
             * Disconnects this input from the connected output
             */
            void disconnectAll() override;
            
            /**
             * Checks wether the input is connected
             */
            bool isConnected() const override { return mInput != nullptr; }
            
        private:
            /*
             * The audio output connected to this input.
             * When it is a nullptr this input is not connected.
             */
            OutputPin* mInput = nullptr;
        };
        
        
        class NAPAPI MultiInputPin : public InputPinBase
        {
        public:
            MultiInputPin() = default;
            
            virtual ~MultiInputPin();
            
            /**
             * This method can be used by the node to pull a buffer of samples for every connected output pin.
             * @return: the vector can contain nullptr items if somewhere down the line of connection silence is returned.
             */
            std::vector<SampleBufferPtr> pull();
            
            void connect(OutputPin& input) override;
            
            void disconnect(OutputPin& input) override;
            
            /**
             * Disconnects this input from all the connected outputs
             */
            void disconnectAll() override;
            
            /**
             * Checks wether the input is connected to any outputs
             */
            bool isConnected() const override { return !mInputs.empty(); }

        private:
            std::set<OutputPin*> mInputs;
        };
        
        
        /**
         * An audio output is used by audio node to connect it to other nodes.
         * The output connects one channel (mono) audio.
         * It outputs a pointer to an owned @SampleBuffer.
         * The PullFunction of this class calls a calculate function on the node it belongs to.
         */
        class NAPAPI OutputPin
        {
            friend class Node;
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
            bool isConnected() const { return !mOutputs.empty(); }
            
        protected:
            /**
             * The buffer containing the latest output
             */
            SampleBuffer mBuffer;
            
        private:
            // Used by @InputPin to poll this output for a new buffer of output samples
            SampleBufferPtr pull();
            
            // Used by the @NodeManager to resize the internal buffers when necessary
            void setBufferSize(int bufferSize);
            
            // The node that owns this output
            Node* mNode = nullptr;
            
            // The inputs that this output is connected to
            // This list is kept so the connections can be broken on destruction.
            std::set<InputPinBase*> mOutputs;
        };
        
    }
    
}
