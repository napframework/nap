/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <iostream>
#include <utility/stringutils.h>
#include <sequenceserviceaudio.h>
#include <sequencetrackaudio.h>
#include <sequenceserviceaudio.h>

// Local Includes
#include "sequenceaudioguiservice.h"
#include "sequenceaudiotrackview.h"


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceAudioGUIService)
RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	static std::vector<std::unique_ptr<rtti::IObjectCreator>(*)(SequenceAudioGUIService*)>& getObjectCreators()
	{
		static std::vector<std::unique_ptr<rtti::IObjectCreator>(*)(SequenceAudioGUIService * service)> vector;
		return vector;
	}


	SequenceAudioGUIService::SequenceAudioGUIService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


	SequenceAudioGUIService::~SequenceAudioGUIService() = default;


	void SequenceAudioGUIService::registerObjectCreators(rtti::Factory& factory)
	{
		for(auto& objectCreator : getObjectCreators())
		{
			factory.addObjectCreator(objectCreator(this));
		}
	}


	bool SequenceAudioGUIService::init(nap::utility::ErrorState& errorState)
	{
		auto* service_gui = getCore().getService<SequenceGUIService>();
		assert(service_gui!= nullptr);

		if(!errorState.check(service_gui->registerTrackTypeForView(RTTI_OF(SequenceTrackAudio), RTTI_OF(SequenceAudioTrackView)),
							 "Error registering track view"))
			return false;

		if(!service_gui->registerTrackViewFactory(RTTI_OF(SequenceAudioTrackView),
												   [](
													    SequenceGUIService& service,
														SequenceEditorGUIView& editorGuiView,
														SequenceEditorGUIState& state)-> std::unique_ptr<SequenceTrackView>
													{
													  return std::make_unique<SequenceAudioTrackView>(service, editorGuiView, state);
													}))
		{
			errorState.fail("Error registering track view factory function");
			return false;
		}

		return true;
	}


	void SequenceAudioGUIService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
	    dependencies.emplace_back(RTTI_OF(SequenceGUIService));
	    dependencies.emplace_back(RTTI_OF(SequenceServiceAudio));
	}
}