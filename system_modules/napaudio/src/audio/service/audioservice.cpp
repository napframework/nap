/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Std includes
#include <iostream>

// Audio includes
#include "audioservice.h"

// Nap includes
#include <nap/logger.h>

// Third party includes
#ifdef NAP_AUDIOFILE_SUPPORT
    #include <mpg123.h>
#endif

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioService)
		RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS


namespace nap
{
	namespace audio
	{
		
		AudioService::AudioService(ServiceConfiguration* configuration) :
				Service(configuration), mNodeManager(mDeletionQueue)
		{
		}
		
		
		bool AudioService::init(nap::utility::ErrorState& errorState)
		{
#ifdef NAP_AUDIOFILE_SUPPORT
            // Initialize mpg123 library
			mpg123_init();
			mMpg123Initialized = true;
#endif
			checkLockfreeTypes();
            return true;
		}


		void AudioService::shutdown()
		{
#ifdef NAP_AUDIOFILE_SUPPORT
			// Close mpg123 library
			if (mMpg123Initialized)
				mpg123_exit();
#endif
		}


		NodeManager& AudioService::getNodeManager()
		{
			return mNodeManager;
		}


		void AudioService::onAudioCallback(float** inputBuffer, float** outputBuffer, unsigned long framesPerBuffer)
		{
			// process the node manager
			mNodeManager.process(inputBuffer, outputBuffer, framesPerBuffer);
			
			// clean the trash bin with nodes and resources that are no longer used and scheduled for destruction
			mDeletionQueue.clear();
		}
		

		void AudioService::checkLockfreeTypes()
		{
            enum EnumType { a, b, c };
            std::atomic<bool> boolVar;
            std::atomic<int> intVar;
            std::atomic<float> floatVar;
            std::atomic<double> doubleVar;
            std::atomic<long> longVar;
            std::atomic<long double> longDoubleVar;
            std::atomic<EnumType> enumVar;

            if (!boolVar.is_lock_free())
                Logger::warn("%s is not lockfree on current platform", "atomic<bool>");
            if (!intVar.is_lock_free())
                Logger::warn("%s is not lockfree on current platform", "atomic<int>");
            if (!floatVar.is_lock_free())
                Logger::warn("%s is not lockfree on current platform", "atomic<float>");
            if (!doubleVar.is_lock_free())
                Logger::warn("%s is not lockfree on current platform", "atomic<double>");
            if (!longVar.is_lock_free())
                Logger::warn("%s is not lockfree on current platform", "atomic<long>");
            if (!longDoubleVar.is_lock_free())
                Logger::warn("%s is not lockfree on current platform", "atomic<long double>");
            if (!enumVar.is_lock_free())
                Logger::warn("%s is not lockfree on current platform", "atomic enum");
		}

		
	}
	
}
