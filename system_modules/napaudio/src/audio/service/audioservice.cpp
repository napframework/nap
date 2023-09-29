/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Std includes
#include <iostream>

// Nap includes
#include <nap/logger.h>
#include <utility/stringutils.h>

// Audio includes
#include "audioservice.h"
#include <audio/resource/audiobufferresource.h>
#include <audio/resource/audiofileresource.h>

// Third party includes
#include <mpg123.h>


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
		
		
		void AudioService::registerObjectCreators(rtti::Factory& factory)
		{
			factory.addObjectCreator(std::make_unique<AudioBufferResourceObjectCreator>(*this));
			factory.addObjectCreator(std::make_unique<AudioFileResourceObjectCreator>(*this));
			factory.addObjectCreator(std::make_unique<MultiAudioFileResourceObjectCreator>(*this));
		}
		
		
		bool AudioService::init(nap::utility::ErrorState& errorState)
		{
			// Initialize mpg123 library
			mpg123_init();
			mMpg123Initialized = true;
			checkLockfreeTypes();
            return true;
		}


		void AudioService::shutdown()
		{
			// Close mpg123 library
			if(mMpg123Initialized)
				mpg123_exit();
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
			/**
			 * Currently this is diabled because atomic<T>::is_lock_free is unavailable in gcc < 4.8
			 */

//            enum EnumType { a, b, c };
//            std::atomic<bool> boolVar;
//            std::atomic<int> intVar;
//            std::atomic<float> floatVar;
//            std::atomic<double> doubleVar;
//            std::atomic<long> longVar;
//            std::atomic<long double> longDoubleVar;
//            std::atomic<EnumType> enumVar;
//
//            if (!boolVar.is_lock_free())
//                Logger::warn("%s is not lockfree on current platform", "atomic<bool>");
//            if (!intVar.is_lock_free())
//                Logger::warn("%s is not lockfree on current platform", "atomic<int>");
//            if (!floatVar.is_lock_free())
//                Logger::warn("%s is not lockfree on current platform", "atomic<float>");
//            if (!doubleVar.is_lock_free())
//                Logger::warn("%s is not lockfree on current platform", "atomic<double>");
//            if (!longVar.is_lock_free())
//                Logger::warn("%s is not lockfree on current platform", "atomic<long>");
//            if (!longDoubleVar.is_lock_free())
//                Logger::warn("%s is not lockfree on current platform", "atomic<long double>");
//            if (!enumVar.is_lock_free())
//                Logger::warn("%s is not lockfree on current platform", "atomic enum");
		}

		
	}
	
}
