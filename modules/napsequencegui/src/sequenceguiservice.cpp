/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <iostream>
#include <utility/stringutils.h>

// Local Includes
#include "sequenceguiservice.h"
#include "sequenceeventtrackview.h"
#include "sequenceeditorgui.h"
#include "sequencetrackcurve.h"
#include "sequencetrackevent.h"
#include "sequencecurvetrackview.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceGUIService)
		RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	static std::vector<std::unique_ptr<rtti::IObjectCreator>(*)(SequenceGUIService*)>& getObjectCreators()
	{
		static std::vector<std::unique_ptr<rtti::IObjectCreator>(*)(SequenceGUIService * service)> vector;
		return vector;
	}


	bool SequenceGUIService::registerObjectCreator(std::unique_ptr<rtti::IObjectCreator>(*objectCreator)(SequenceGUIService* service))
	{
		getObjectCreators().emplace_back(objectCreator);
		return true;
	}


	SequenceGUIService::SequenceGUIService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


	SequenceGUIService::~SequenceGUIService() = default;


	void SequenceGUIService::registerObjectCreators(rtti::Factory& factory)
	{
		for(auto& objectCreator : getObjectCreators())
		{
			factory.addObjectCreator(objectCreator(this));
		}

		factory.addObjectCreator(std::make_unique<SequenceEditorGUIObjectCreator>(*this));
	}


	bool SequenceGUIService::init(nap::utility::ErrorState& errorState)
	{
		if(!errorState.check(registerEventView<std::string>(), "Error registering event view"))
			return false;

		if(!errorState.check(registerEventView<int>(), "Error registering event view"))
			return false;

		if(!errorState.check(registerEventView<float>(), "Error registering event view"))
			return false;

		if(!errorState.check(registerEventView<glm::vec2>(), "Error registering event view"))
			return false;

		if(!errorState.check(registerEventView<glm::vec3>(), "Error registering event view"))
			return false;

		if(!errorState.check(registerTrackTypeForView(RTTI_OF(SequenceTrackEvent), RTTI_OF(SequenceEventTrackView)),
							  "Error registering track view"))
			return false;

		if(!errorState.check(registerTrackTypeForView(RTTI_OF(SequenceTrackCurve<float>), RTTI_OF(SequenceCurveTrackView)),
							 "Error registering track view"))
			return false;

		if(!errorState.check(registerTrackTypeForView(RTTI_OF(SequenceTrackCurve<glm::vec2>), RTTI_OF(SequenceCurveTrackView)),
							 "Error registering track view"))
			return false;

		if(!errorState.check(registerTrackTypeForView(RTTI_OF(SequenceTrackCurve<glm::vec3>), RTTI_OF(SequenceCurveTrackView)),
							 "Error registering track view"))
			return false;

		if(!registerTrackViewFactory(RTTI_OF(SequenceCurveTrackView), 	[](	SequenceGUIService& service,
																			SequenceEditorGUIView& editorGuiView,
																			SequenceEditorGUIState& state)-> std::unique_ptr<SequenceTrackView>
																			{
																				return std::make_unique<SequenceCurveTrackView>(service, editorGuiView, state);
																			}))
		{
			errorState.fail("Error registering track view factory function");
			return false;
		}

		if(!registerTrackViewFactory(RTTI_OF(SequenceEventTrackView), 	[](	SequenceGUIService& service,
																	  		SequenceEditorGUIView& editorGuiView,
																	  		SequenceEditorGUIState& state)-> std::unique_ptr<SequenceTrackView>
									  									{
										  									return std::make_unique<SequenceEventTrackView>(service, editorGuiView, state);
									  									}))
		{
			errorState.fail("Error registering track view factory function");
			return false;
		}

		return true;
	}


	const SequenceEventTrackSegmentViewFactoryMap& SequenceGUIService::getEventSegmentViewFactory() const
	{
		return mEventSegmentViewFactoryMap;
	}


	const std::vector<rtti::TypeInfo>& SequenceGUIService::getRegisteredSegmentEventTypes() const
	{
		return mSegmentEventTypes;
	}


	bool SequenceGUIService::registerTrackViewFactory(rtti::TypeInfo trackType, SequenceTrackViewFactoryFunc factory)
	{
		assert(mTrackViewFactoryMap.find(trackType)==mTrackViewFactoryMap.end()); // duplicate entry
		return mTrackViewFactoryMap.emplace(trackType, factory).second;
	}


	const SequenceTrackViewFactoryMap& SequenceGUIService::getTrackViewFactory() const
	{
		return mTrackViewFactoryMap;
	}


	template<typename T>
	bool SequenceGUIService::registerEventView()
	{
		// register type of view
		assert(std::find(mSegmentEventTypes.begin(), mSegmentEventTypes.begin() + mSegmentEventTypes.size(), RTTI_OF(SequenceTrackSegmentEvent<T>)) == mSegmentEventTypes.end()); // type already added
		mSegmentEventTypes.emplace_back(RTTI_OF(SequenceTrackSegmentEvent<T>));

		// register view
		auto segment_it = mEventSegmentViewFactoryMap.find(RTTI_OF(SequenceTrackSegmentEvent<T>));
		assert(segment_it== mEventSegmentViewFactoryMap.end()); // type already registered
		if(segment_it== mEventSegmentViewFactoryMap.end())
		{
			mEventSegmentViewFactoryMap.emplace(RTTI_OF(SequenceTrackSegmentEvent<T>), []()->std::unique_ptr<SequenceEventTrackSegmentView<T>>{ return std::make_unique<SequenceEventTrackSegmentView<T>>(); });
		}

		// register popup action handler
		auto event_it = mEditEventHandlerMap.find(RTTI_OF(SequenceGUIActions::OpenEditEventSegmentPopup<T>));
		assert(event_it== mEditEventHandlerMap.end()); // type already registered
		if(event_it== mEditEventHandlerMap.end())
		{
			mEditEventHandlerMap.emplace(RTTI_OF(SequenceGUIActions::OpenEditEventSegmentPopup<T>), [](SequenceEventTrackView& view){ view.template handleEditEventSegmentPopup<T>(); });
		}

		event_it = mEditEventHandlerMap.find(RTTI_OF(SequenceGUIActions::EditingEventSegment<T>));
		assert(event_it== mEditEventHandlerMap.end()); // type already registered
		if(event_it== mEditEventHandlerMap.end())
		{
			mEditEventHandlerMap.emplace(RTTI_OF(SequenceGUIActions::EditingEventSegment<T>), [](SequenceEventTrackView& view){ view.template handleEditEventSegmentPopup<T>(); });
		}

		// register paste handler
		auto& handler_paste_events = mPastEventMap;

		auto paste_it = handler_paste_events.find(RTTI_OF(SequenceTrackSegmentEvent<T>));
		assert(paste_it == handler_paste_events.end()); // type already registered
		if(paste_it == handler_paste_events.end())
		{
			handler_paste_events.emplace(RTTI_OF(SequenceTrackSegmentEvent<T>), [](	SequenceEventTrackView& view,
																					   const std::string& trackID,
																					   const SequenceTrackSegmentEventBase& eventBase,
																					   double time)
			{
			  	view.template pasteEvent<SequenceTrackSegmentEvent<T>>(trackID, eventBase, time);
			});
		}

		return true;
	}


	bool SequenceGUIService::registerTrackTypeForView(rtti::TypeInfo trackType, rtti::TypeInfo viewType)
	{
		assert(mTrackViewTypeMap.find(trackType)==mTrackViewTypeMap.end()); // duplicate entry
		return mTrackViewTypeMap.emplace(trackType, viewType).second;
	}


	rtti::TypeInfo SequenceGUIService::getViewTypeForTrackType(rtti::TypeInfo trackType) const
	{
		assert(mTrackViewTypeMap.find(trackType)!=mTrackViewTypeMap.end()); // entry not found
		return mTrackViewTypeMap.find(trackType)->second;
	}


	void SequenceGUIService::invokeEditEventHandler(rtti::TypeInfo eventType, SequenceEventTrackView& view) const
	{
		assert(mEditEventHandlerMap.find(eventType)!=mEditEventHandlerMap.end());
		mEditEventHandlerMap.find(eventType)->second(view);
	}


	void SequenceGUIService::invokePasteEvent(	rtti::TypeInfo eventType,
											    SequenceEventTrackView& view,
											    const std::string& trackID,
											    const SequenceTrackSegmentEventBase& eventBase,
											    double time) const
	{
		assert(mPastEventMap.find(eventType)!=mPastEventMap.end());
		mPastEventMap.find(eventType)->second(view, trackID, eventBase, time);
	}


	std::vector<rtti::TypeInfo> SequenceGUIService::getAllTrackTypes() const
	{
		std::vector<rtti::TypeInfo> track_types;
		for(const auto& it : mTrackViewTypeMap)
		{
			track_types.emplace_back(it.first);
		}
		return track_types;
	}


	std::vector<rtti::TypeInfo> SequenceGUIService::getAllRegisteredEventActions() const
	{
		std::vector<rtti::TypeInfo> event_actions;
		for(const auto& it : mEditEventHandlerMap)
		{
			event_actions.emplace_back(it.first);
		}
		return event_actions;
	}
}