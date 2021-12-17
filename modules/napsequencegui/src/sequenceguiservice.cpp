/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <iostream>
#include <utility/stringutils.h>
#include <sequenceservice.h>
#include <imguiservice.h>

// Local Includes
#include "sequenceguiservice.h"
#include "sequenceeventtrackview.h"
#include "sequenceeditorgui.h"
#include "sequencetrackcurve.h"
#include "sequencetrackevent.h"
#include "sequencecurvetrackview.h"
#include "sequencetrackview.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceGUIService)
RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Icons
	//////////////////////////////////////////////////////////////////////////

	namespace icon
	{
		namespace sequencer
		{
			static const std::vector<std::string>& get()
			{
				const static std::vector<std::string> map =
				{
					icon::sequencer::play,
					icon::sequencer::stop,
					icon::sequencer::rewind,
					icon::sequencer::up,
					icon::sequencer::down,
					icon::sequencer::pause,
					icon::sequencer::unpause,
					icon::sequencer::plus,
					icon::sequencer::minus
				};
				return map;
			}
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// Object Creators
	//////////////////////////////////////////////////////////////////////////

	static std::vector<std::unique_ptr<rtti::IObjectCreator>(*)(SequenceGUIService*)>& getObjectCreators()
	{
		static std::vector<std::unique_ptr<rtti::IObjectCreator>(*)(SequenceGUIService * service)> vector;
		return vector;
	}


	//////////////////////////////////////////////////////////////////////////
	// SequenceGUIService
	//////////////////////////////////////////////////////////////////////////

	bool SequenceGUIService::registerObjectCreator(std::unique_ptr<rtti::IObjectCreator>(*objectCreator)(SequenceGUIService* service))
	{
		getObjectCreators().emplace_back(objectCreator);
		return true;
	}


	SequenceGUIService::SequenceGUIService(ServiceConfiguration* configuration) :
		Service(configuration)
	{ }


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
		// Get gui service and colors
		mGuiService = getCore().getService<IMGuiService>();
		mColors.init(mGuiService->getColors());

		// Load all icons
		const auto& icon_names = icon::sequencer::get();
		for (const auto& icon_name : icon_names)
		{
			if (!mGuiService->loadIcon(icon_name, this->getModule(), errorState))
				return false;
		}

		// Register all views
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

		// Register track types
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


	const std::vector<rtti::TypeInfo>& SequenceGUIService::getRegisteredSegmentEventTypes() const
	{
		return mSegmentEventTypes;
	}


	bool SequenceGUIService::registerTrackViewFactory(rtti::TypeInfo trackType, SequenceTrackViewFactoryFunc factory)
	{
		assert(mTrackViewFactoryMap.find(trackType)==mTrackViewFactoryMap.end()); // duplicate entry
		return mTrackViewFactoryMap.emplace(trackType, factory).second;
	}


	std::unique_ptr<SequenceTrackView> SequenceGUIService::invokeTrackViewFactory(
		rtti::TypeInfo viewType,
		SequenceEditorGUIView& view,
		SequenceEditorGUIState& state)
	{
		assert(mTrackViewFactoryMap.find(viewType) != mTrackViewFactoryMap.end()); // entry not found
		return mTrackViewFactoryMap.find(viewType)->second(*this, view, state);
	}


	std::unique_ptr<SequenceEventTrackSegmentViewBase> SequenceGUIService::invokeEventTrackSegmentViewFactory(rtti::TypeInfo eventType)
	{
		assert(mEventSegmentViewFactoryMap.find(eventType) != mEventSegmentViewFactoryMap.end()); // entry not found
		return mEventSegmentViewFactoryMap.find(eventType)->second();
	}


	template<typename T>
	bool SequenceGUIService::registerEventView()
	{
		if(std::find(mSegmentEventTypes.begin(), mSegmentEventTypes.begin() + mSegmentEventTypes.size(), RTTI_OF(SequenceTrackSegmentEvent<T>)) != mSegmentEventTypes.end())
			return false;

		mSegmentEventTypes.emplace_back(RTTI_OF(SequenceTrackSegmentEvent<T>));

		// register view
		auto segment_it = mEventSegmentViewFactoryMap.find(RTTI_OF(SequenceTrackSegmentEvent<T>));
		assert(segment_it== mEventSegmentViewFactoryMap.end()); // type already registered
		mEventSegmentViewFactoryMap.emplace(RTTI_OF(SequenceTrackSegmentEvent<T>), []()->std::unique_ptr<SequenceEventTrackSegmentViewBase>{ return std::make_unique<SequenceEventTrackSegmentView<T>>(); });

		// register popup action handler
		auto event_it = mEditEventHandlerMap.find(RTTI_OF(SequenceGUIActions::OpenEditEventSegmentPopup<T>));
		assert(event_it== mEditEventHandlerMap.end()); // type already registered
		mEditEventHandlerMap.emplace(RTTI_OF(SequenceGUIActions::OpenEditEventSegmentPopup<T>), [](SequenceEventTrackView& view){ view.template handleEditEventSegmentPopup<T>(); });

		event_it = mEditEventHandlerMap.find(RTTI_OF(SequenceGUIActions::EditingEventSegment<T>));
		assert(event_it== mEditEventHandlerMap.end()); // type already registered
		mEditEventHandlerMap.emplace(RTTI_OF(SequenceGUIActions::EditingEventSegment<T>), [](SequenceEventTrackView& view){ view.template handleEditEventSegmentPopup<T>(); });

		// register paste handler
		auto& handler_paste_events = mPasteEventMap;

		auto paste_it = handler_paste_events.find(RTTI_OF(SequenceTrackSegmentEvent<T>));
		assert(paste_it == handler_paste_events.end()); // type already registered
		handler_paste_events.emplace(RTTI_OF(SequenceTrackSegmentEvent<T>), [](SequenceEventTrackView& view,
			const std::string& trackID,
			const SequenceTrackSegmentEventBase& eventBase,
			double time)
		{
			view.template pasteEvent<SequenceTrackSegmentEvent<T>>(trackID, eventBase, time);
		});
		
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
		assert(mPasteEventMap.find(eventType)!= mPasteEventMap.end());
		mPasteEventMap.find(eventType)->second(view, trackID, eventBase, time);
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


	void SequenceGUIService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
	    dependencies.emplace_back(RTTI_OF(SequenceService));
		dependencies.emplace_back(RTTI_OF(IMGuiService));
	}


	nap::IMGuiService& SequenceGUIService::getGui()
	{
		assert(mGuiService != nullptr);
		return *mGuiService;
	}


	void SequenceGUIService::Colors::init(const IMGuiColorPalette& palette)
	{
		mHigh = ImGui::ColorConvertFloat4ToU32(ImVec4(palette.mHighlightColor));
		mDark = ImGui::ColorConvertFloat4ToU32(ImVec4(palette.mDarkColor));
		mFro3 = ImGui::ColorConvertFloat4ToU32(ImVec4(palette.mFront3Color));
		mFro2 = ImGui::ColorConvertFloat4ToU32(ImVec4(palette.mFront2Color));
		mFro1 = ImGui::ColorConvertFloat4ToU32(ImVec4(palette.mFront1Color));
		mBack = ImGui::ColorConvertFloat4ToU32(ImVec4(palette.mBackgroundColor));
	}
}
