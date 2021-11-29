/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <iostream>
#include <utility/stringutils.h>
#include <parametervec.h>
#include <parameternumeric.h>

// NAP Includes
#include <sequenceservice.h>

// Local Includes
#include "sequenceserviceaudio.h"
#include "sequencetrackaudio.h"
#include "sequencecontrolleraudio.h"
#include "sequenceplayeraudiooutput.h"
#include "sequenceplayeraudioadapter.h"
#include "sequenceplayeraudioclock.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceServiceAudio)
RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	SequenceServiceAudio::SequenceServiceAudio(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


	SequenceServiceAudio::~SequenceServiceAudio() = default;


	void SequenceServiceAudio::registerObjectCreators(rtti::Factory& factory)
	{
		auto* sequence_service = getCore().getService<SequenceService>();
		assert(sequence_service!= nullptr);

		factory.addObjectCreator(std::make_unique<SequencePlayerAudioOutputObjectCreator>(*sequence_service));
		factory.addObjectCreator(std::make_unique<SequencePlayerAudioClockObjectCreator>(*this));
	}


	bool SequenceServiceAudio::init(nap::utility::ErrorState& errorState)
	{
		auto* sequence_service = getCore().getService<SequenceService>();
		assert(sequence_service!= nullptr);

		if(!errorState.check(sequence_service->registerControllerTypeForTrackType(RTTI_OF(SequenceTrackAudio),
                                                                                  RTTI_OF(SequenceControllerAudio)),
                             "Could not register controller type for track type"))
			return false;

		if(!errorState.check(sequence_service->registerControllerFactoryFunc(RTTI_OF(SequenceControllerAudio),
																			  [sequence_service](SequencePlayer& player, SequenceEditor& editor)->std::unique_ptr<SequenceController>
																			  {
																				  return std::make_unique<SequenceControllerAudio>(*sequence_service, player, editor);
																			  }), "Error registering controller factory function"))
			return false;

		if(!errorState.check(sequence_service->registerAdapterFactoryFunc(RTTI_OF(SequenceTrackAudio),
																		  [](const SequenceTrack& track, SequencePlayerOutput& output, const SequencePlayer& player)->std::unique_ptr<SequencePlayerAdapter>
																			{
																			  assert(output.get_type() == RTTI_OF(SequencePlayerAudioOutput)); // type mismatch

																			  auto& audio_output = static_cast<SequencePlayerAudioOutput&>(output);

																			  auto adapter = std::make_unique<SequencePlayerAudioAdapter>(track, audio_output, player);

																			  return std::move(adapter);
																			}), "Error registering adapter factory function"))
			return false;

		return true;
	}


	void SequenceServiceAudio::update(double deltaTime)
	{
	}


	void SequenceServiceAudio::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
	    dependencies.emplace_back(RTTI_OF(SequenceService));
	    dependencies.emplace_back(RTTI_OF(audio::AudioService));
	}
}