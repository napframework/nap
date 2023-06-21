/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <mutex>
#include <set>

// Nap includes
#include <utility/threading.h>
#include <nap/signalslot.h>

// Audio includes
#include <audio/utility/audiotypes.h>
#include <audio/core/process.h>

namespace nap
{
	namespace audio
	{
		
		// Forward declarations
		class Node;
		
		/**
		 * The audio node manager represents a node system for audio processing.
		 * The nodes in the system can have multiple inputs and outputs that can be connected between different nodes.
		 * A connection represents a mono audio signal.
		 * Does not own the nodes but maintains a list of existing nodes that is updated from the node's constructor end destructors.
		 */
		class NAPAPI NodeManager final
		{
			friend class Node;
			friend class OutputNode;
			friend class InputNode;
		
		public:
			using OutputMapping = std::vector<std::vector<SampleBuffer*>>;
		
		public:
			NodeManager(DeletionQueue& deletionQueue) : mDeletionQueue(deletionQueue) { }
			
			~NodeManager();
			
			/**
			 * This function is typically called by an audio callback to perform all the audio processing.
			 * It feeds the inputbuffer to the audio input nodes and polls the audio output nodes for output.
			 * @param inputBuffer: an array of float arrays, representing one sample buffer for every channel
			 * @param outputBuffer: an array of float arrays, representing one sample buffer for every channel
			 * @param framesPerBuffer: the number of samples that has to be processed per channel
			 */
			void process(float** inputBuffer, float** outputBuffer, unsigned long framesPerBuffer);
			
			/**
			 * This function is typically called by an audio callback to perform all the audio processing.
			 * It feeds the inputbuffer to the audio input nodes and polls the audio output nodes for output.
			 * @param inputBuffer: a vector of sample buffers, representing one sample buffer for every channel
			 * @param outputBuffer: a vector of sample buffers, representing one sample buffer for every channel
			 * @param framesPerBuffer: the number of samples that has to be processed per channel
			 */
			void process(std::vector<SampleBuffer*>& inputBuffer, std::vector<SampleBuffer*>& outputBuffer,
			             unsigned long framesPerBuffer);
			
			/**
			 * This signal is emitted from the @process() function after processing every internal buffer of audio.
			 */
			Signal<DiscreteTimeValue> mUpdateSignal;
			
			/**
			 * Enqueue a lambda to be executed before the processing of the next internal buffer starts.
			 * This way modifications to the processing chain can be made in a threadsafe manner from outside of the audio thread, with a timing accuracy that corresponds to the internal buffer size.
			 * @param task Lamba without arguments that will be called on the next audio callback
			 */
			void enqueueTask(nap::TaskQueue::Task task);
			
			/**
			 * @return: the number of input channels that will be fed into the node system
			 */
			int getInputChannelCount() const { return mInputChannelCount; }
			
			/**
			 * @return: the number of output channels that will be processed by the node system
			 */
			int getOutputChannelCount() const { return mOutputChannelCount; }
			
			/**
			 * @return: the sample rate that the node system runs on
			 */
			float getSampleRate() const { return mSampleRate; }
			
			/**
			 * @return: the number of audio samples needed per millisecond (TimeValue unit)
			 */
			float getSamplesPerMillisecond() const { return mSamplesPerMillisecond; }
			
			/**
			 * @return the buffer size the node system is running on.
			 * Beware: this can be smaller than the buffersize the audio device is running on.
			 * The latter is specified by the framesPerBuffer parameter in the process() function.
			 * This number indicates the size of each buffer that the nodes' process() functions have to present.
			 */
			int getInternalBufferSize() const { return mInternalBufferSize; }
			
			/**
			 * @return the absolute time in samples
			 */
			const DiscreteTimeValue& getSampleTime() const { return mSampleTime; }
			
			/**
			 * Sets the number of input channels that will be fed into the node system
			 * @param inputChannelCount the number of input channels
			 */
			void setInputChannelCount(int inputChannelCount);
			
			/**
			 * Sets the number of output channels that will be processed by the node system
			 * @Param outputChannelCount the number of output channels that the node system provides audio to
			 */
			void setOutputChannelCount(int outputChannelCount);
			
			/**
			 * Changes the sample rate the node system is running on. This method is normally called by the AudioService.
			 * @param sampleRate the sample rate in Hz
			 */
			void setSampleRate(float sampleRate);
			
			/**
			 * Changes the internal buffer size that the node system uses.
			 * The internal buffer size determines the timing accuracy of state changes within the node system.
			 * The internal buffer size needs to fit a discrete amount of times into the audio callback's buffer size.
			 * Beware: this can be smaller than the buffersize the audio device is running on.
			 * @param size the new buffer size
			 */
			void setInternalBufferSize(int size);
			
			/**
			 * Used by nodes to register themselves to be processed directly by the node manager
			 * @param rootProcess The root process is a process or node that is executed on every audio callback without being connected to an input of another node.
			 * In most cases the root process is an OutputNode.
			 */
			void registerRootProcess(Process& rootProcess);
			
			
			/**
			 * Used by nodes to unregister themselves to be processed directly by the node manager.
			 * @param rootProcess The root process is a process or node that is executed on every audio callback without being connected to an input of another node.
			 * In most cases the root process is an OutputNode.
			 */
			void unregisterRootProcess(Process& rootProcess);
			
			/**
			 * Constructs an object managed by a SafeOwner that will dispose the object in the NodeManager's DeletionQueue when it is no longer used.
			 * This will make sure the object is always destructed by the audio thread, so after going out of scope it can be safely referred until the next audio callback cycle.
			 * @tparam T The type of the object retained by the newly constructed SafeOwner.
			 * @param args The arguments passed to the constructor of the object that will be retained by the newly created SafeOwner
			 * @return A SafeOwner retaining the new object.
			 */
			template<typename T, typename... Args>
			SafeOwner<T> makeSafe(Args&& ... args)
			{
				auto owner = SafeOwner<T>(mDeletionQueue, new T(std::forward<Args>(args)...));
				return owner;
			}
			
			/**
			 *  Make an object to be retained by a SafeOwner that will dispose the object in the NodeManager's DeletionQueue when it is no longer used.
			  * This will make sure the object is always destructed by the audio thread, so after going out of scope it can be safely referred until the next audio callback cycle.
			 * @tparam T The type of the object to be retained by the newly constructed SafeOwner
			 * @param ptr Pointer to the object to be retained
			 * @return A SafeOwner retaining the passed object.
			 */
			template<typename T>
			SafeOwner<T> makeSafe(T* ptr)
			{
				auto owner = SafeOwner<T>(mDeletionQueue, ptr);
				return owner;
			}
			
			/**
			 * Returns the DeletionQueue that this node manager uses to construct and destruct nodes or other processes on the audio thread safely.
			 */
			DeletionQueue& getDeletionQueue() { return mDeletionQueue; }
			
			/**
			 * Signal triggered whenever the input or output channel count of the node manager changes
			 */
			Signal<NodeManager&> mChannelCountChangedSignal;
		
		
		private:
			// Used by the nodes to register themselves on construction
			void registerNode(Node& node);
			
			// Used by the nodes to unregister themselves on destrction
			void unregisterNode(Node& node);
		
		private:
			/*
			 * Used by @OutputNode to provide new output for the node system
			 * Note: multiple output buffers can be provided for the same channel by different output nodes.
			 * @param buffer: a buffer of output
			 * @param channel: the channel that the output will be played on
			 */
			void provideOutputBufferForChannel(SampleBuffer* buffer, int channel);
			
			/*
			 * Used by @InputNode to request audio input for the current buffer
			 * @param channel: the input channel that is being monitored
			 * @param index: the index of the requested sample within the buffer currently being calculated.
			 * TODO: optimize by returning a float array for the whole buffer
			 */
			const SampleValue& getInputSample(int channel, int index) const
			{
				return mInputBuffer[channel][mInternalBufferOffset + index];
			}
			
			int mInputChannelCount = 0; // Number of input channels this node manager processes
			int mOutputChannelCount = 0; // Number of channel this node manager outputs
			float mSampleRate = 0; // Current sample rate the node manager runs on.
			float mSamplesPerMillisecond = 0; // cached here for optimization purpose
			int mInternalBufferSize = 64; // The internal buffersize that the node manager runs on.
			
			DiscreteTimeValue mSampleTime = 0; // the actual sample time clock of the audio system
			unsigned int mInternalBufferOffset = 0; // helper variable for the process method
			
			// for each output channel a vector of buffers that need to be played back on the corresponding channel
			OutputMapping mOutputMapping;
			
			std::vector<float*> mInputBuffer; //  Pointing to the audio input that this node manager has to process. The format is a non-interleaved array containing a float array for each channel.
			
			std::set<Node*> mNodes; // all the audio nodes managed by this node manager
			std::set<Process*> mRootProcesses; // the nodes that will be processed directly by the manager on every audio callback
			
			nap::TaskQueue mTaskQueue = { 256 }; // Queue with lambda functions to be executed before processing the next itnernal buffer.
			DeletionQueue& mDeletionQueue; // Deletion queue used to safely create and destruct nodes in a threadsafe manner.
		};
		
	}
}
