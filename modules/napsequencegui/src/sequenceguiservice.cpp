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
		if( !registerEventTrackSegmentView(	RTTI_OF(SequenceEventTrackSegmentView<std::string>),
									  		std::make_unique<SequenceEventTrackSegmentView<std::string>>()) ||
			!registerEventTrackSegmentView(	RTTI_OF(SequenceEventTrackSegmentView<int>),
											  std::make_unique<SequenceEventTrackSegmentView<int>>()) ||
			!registerEventTrackSegmentView(	RTTI_OF(SequenceEventTrackSegmentView<float>),
											  std::make_unique<SequenceEventTrackSegmentView<float>>()) ||
			!registerEventTrackSegmentView(	RTTI_OF(SequenceEventTrackSegmentView<glm::vec2>),
											  std::make_unique<SequenceEventTrackSegmentView<glm::vec2>>()) ||
			!registerEventTrackSegmentView(	RTTI_OF(SequenceEventTrackSegmentView<glm::vec3>),
											  std::make_unique<SequenceEventTrackSegmentView<glm::vec3>>()))
		{
			errorState.fail("Error creating event segment views");
			return false;
		}

		registerEventView<std::string>();
		registerEventView<int>();
		registerEventView<float>();
		registerEventView<glm::vec2>();
		registerEventView<glm::vec3>();

		registerTrackTypeForView(RTTI_OF(SequenceTrackEvent), RTTI_OF(SequenceEventTrackView));
		registerTrackTypeForView(RTTI_OF(SequenceTrackCurve<float>), RTTI_OF(SequenceCurveTrackView));
		registerTrackTypeForView(RTTI_OF(SequenceTrackCurve<glm::vec2>), RTTI_OF(SequenceCurveTrackView));
		registerTrackTypeForView(RTTI_OF(SequenceTrackCurve<glm::vec3>), RTTI_OF(SequenceCurveTrackView));
		//registerTrackTypeForView(RTTI_OF(SequenceTrackCurve<glm::vec4>), RTTI_OF(SequenceCurveTrackView));

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


	bool SequenceGUIService::registerEventTrackSegmentView(rtti::TypeInfo typeInfo, std::unique_ptr<SequenceEventTrackSegmentViewBase> view)
	{
		assert(mEventSegmentViews.find(typeInfo)==mEventSegmentViews.end()); // duplicate entry
		return mEventSegmentViews.emplace(typeInfo, std::move(view)).second;
	}


	void SequenceGUIService::update(double deltaTime)
	{
	}


	const SequenceEventTrackSegmentViewMap& SequenceGUIService::getEventSegmentViews() const
	{
		return mEventSegmentViews;
	}


	const std::vector<rtti::TypeInfo>& SequenceGUIService::getRegisteredEventTypes() const
	{
		return mEventTypes;
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
		assert(std::find(mEventTypes.begin(), mEventTypes.begin() + mEventTypes.size(), RTTI_OF(SequenceTrackSegmentEvent<T>)) == mEventTypes.end()); // type already added
		mEventTypes.emplace_back(RTTI_OF(SequenceTrackSegmentEvent<T>));

		// register view
		auto segment_it = mEventSegmentViews.find(RTTI_OF(SequenceTrackSegmentEvent<T>));
		assert(segment_it==mEventSegmentViews.end()); // type already registered
		if(segment_it==mEventSegmentViews.end())
		{
			mEventSegmentViews.emplace(RTTI_OF(SequenceTrackSegmentEvent<T>), std::make_unique<SequenceEventTrackSegmentView<T>>());
		}

		// register popup action handler
		auto& handle_edit_events = mEditEventHandlerMap;

		auto event_it = handle_edit_events.find(RTTI_OF(SequenceGUIActions::OpenEditEventSegmentPopup<T>));
		assert(event_it== handle_edit_events.end()); // type already registered
		if(event_it== handle_edit_events.end())
		{
			handle_edit_events.emplace(RTTI_OF(SequenceGUIActions::OpenEditEventSegmentPopup<T>), &SequenceEventTrackView::handleEditEventSegmentPopup<T> );
		}

		event_it = handle_edit_events.find(RTTI_OF(SequenceGUIActions::EditingEventSegment<T>));
		assert(event_it== handle_edit_events.end()); // type already registered
		if(event_it== handle_edit_events.end())
		{
			handle_edit_events.emplace(RTTI_OF(SequenceGUIActions::EditingEventSegment<T>), &SequenceEventTrackView::handleEditEventSegmentPopup<T> );
		}

		// register paste handler
		auto& handler_paste_events = mPastEventMap;

		auto paste_it = handler_paste_events.find(RTTI_OF(SequenceTrackSegmentEvent<T>));
		assert(paste_it == handler_paste_events.end()); // type already registered
		if(paste_it == handler_paste_events.end())
		{
			handler_paste_events.emplace(RTTI_OF(SequenceTrackSegmentEvent<T>), &SequenceEventTrackView::pasteEvent<SequenceTrackSegmentEvent<T>> );
		}

		return true;
	}


	bool SequenceGUIService::registerTrackTypeForView(rtti::TypeInfo trackType, rtti::TypeInfo viewType)
	{
		assert(mTrackViewTypeMap.find(trackType)==mTrackViewTypeMap.end()); // duplicate entry
		return mTrackViewTypeMap.emplace(trackType, viewType).second;
	}


	const SequenceTrackTypeForViewTypeMap& SequenceGUIService::getTrackTypeForViewTypeMap() const
	{
		return mTrackViewTypeMap;
	}


	const std::unordered_map<rtti::TypeInfo,  SequenceEventTrackEditEventMemFun>& SequenceGUIService::getEditEventHandlerMap() const
	{
		return mEditEventHandlerMap;
	}


	const std::unordered_map<rttr::type,SequenceEventTrackPasteEventMemFun>& SequenceGUIService::getPasteEventMap() const
	{
		return mPastEventMap;
	}
}