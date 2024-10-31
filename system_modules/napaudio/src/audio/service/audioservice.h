/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Audio includes
#include <audio/core/audionodemanager.h>
#include <audio/utility/safeptr.h>

// Nap includes
#include <nap/service.h>
#include <utility/threading.h>

namespace nap
{
	namespace audio
	{
		// Forward declarations
		class AudioService;

		/**
		 * Service that provides audio input and output processing directly for hardware audio devices.
		 * Provides static methods to poll the current system for available audio devices using portaudio.
		 */
		class NAPAPI AudioService final : public Service
		{
		RTTI_ENABLE(nap::Service)

		public:
			AudioService(ServiceConfiguration* configuration);

			~AudioService() = default;

			/**
			 * Initializes portaudio.
			 */
			bool init(nap::utility::ErrorState& errorState) override;

			/**
			 * Called on shutdown of the service. Closes portaudio stream and shuts down portaudio.
			 */
			 void shutdown() override;

			/**
			 * @return the audio node manager owned by the audio service. The node manager contains a node system that performs all the DSP.
			 */
			NodeManager& getNodeManager();

			/**
             * This function is typically called by a hardware callback from the device to perform all the audio processing.
             * It performs memory management and processes a lockfree event queue before it invokes the NodeManager::process() to process audio.
             * @param inputBuffer: an array of float arrays, representing one sample buffer for every channel
             * @param outputBuffer: an array of float arrays, representing one sample buffer for every channel
             * @param framesPerBuffer: the number of samples that has to be processed per channel
             */
			void onAudioCallback(float** inputBuffer, float** outputBuffer, unsigned long framesPerBuffer);

			/**
			 * Enqueue a task to be executed within the process() method for thread safety
			 */
			void enqueueTask(TaskQueue::Task task) { mNodeManager.enqueueTask(task); }

        private:
			/*
			 * Checks wether certain atomic types that are used within the library are lockfree and gives a warning if not.
			 */
			void checkLockfreeTypes();

		private:
			NodeManager mNodeManager; // The node manager that performs the audio processing.
			bool mMpg123Initialized	   = false;	// If mpg123 is initialized

			// DeletionQueue with nodes that are no longer used and that can be cleared and destructed safely on the next audio callback.
			// Clearing is performed on the audio callback to make sure the node can not be destructed while it is being processed.
			DeletionQueue mDeletionQueue;
		};
	}
}
