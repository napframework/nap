/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/service.h>
#include <entity.h>
#include <nap/datetime.h>
#include <rtti/factory.h>

namespace nap
{
	class SequenceEventTrackSegmentViewBase;
	class SequenceEventTrackView;
	class SequenceEventTrackView;
	class SequenceTrackSegmentEventBase;
	class SequenceTrackView;
	class SequenceGUIService;
	class SequenceEditorGUIView;
	class SequenceEditorGUIState;

	using SequenceTrackViewFactoryFunc 			= std::function<std::unique_ptr<SequenceTrackView>(SequenceGUIService&, SequenceEditorGUIView&, SequenceEditorGUIState&)>;
	using SequenceEventTrackSegmentViewMap 		= std::unordered_map<rtti::TypeInfo, std::unique_ptr<SequenceEventTrackSegmentViewBase>>;
	using SequenceTrackViewFactoryMap 			= std::unordered_map<rtti::TypeInfo, SequenceTrackViewFactoryFunc>;
	using SequenceTrackViewMap 					= std::unordered_map<rtti::TypeInfo, std::unique_ptr<SequenceTrackView>>;
	using SequenceTrackTypeForViewTypeMap		= std::unordered_map<rtti::TypeInfo, rtti::TypeInfo>;
	using SequenceEventTrackPasteEventMemFun 	= void (SequenceEventTrackView::*)(const std::string&, const SequenceTrackSegmentEventBase&, double);
	using SequenceEventTrackEditEventMemFun 	= void (SequenceEventTrackView::*)();

	class NAPAPI SequenceGUIService : public Service
	{
		friend class SequencePlayerOutput;

		RTTI_ENABLE(Service)
	public:
		/**
		 * Constructor
		 */
		SequenceGUIService(ServiceConfiguration* configuration);

		/**
		 * Deconstructor
		 */
		~SequenceGUIService() override;

		/**
		 * registers object creator method that can be passed on to the rtti factory
		 * @param objectCreator unique pointer to method
		 */
		static bool registerObjectCreator(std::unique_ptr<rtti::IObjectCreator>(*objectCreator)(SequenceGUIService*));

		/**
		 * call this static method to register you a custom view for a custom event type
		 * T is the value type of the event ( SequenceEvent<T> )
		 * @tparam T value to of the event
		 * @return true when called
		 */
		template<typename T>
		bool registerEventView();

		bool registerEventTrackSegmentView(rtti::TypeInfo typeInfo, std::unique_ptr<SequenceEventTrackSegmentViewBase> view);

		bool registerTrackTypeForView(rtti::TypeInfo trackType, rtti::TypeInfo viewType);

		bool registerTrackViewFactory(rtti::TypeInfo trackType, SequenceTrackViewFactoryFunc factory);

		const SequenceEventTrackSegmentViewMap& getEventSegmentViews() const;

		const std::vector<rtti::TypeInfo>& getEventTypes() const;

		const SequenceTrackViewFactoryMap& getTrackViewFactory() const;

		const SequenceTrackTypeForViewTypeMap& getTrackTypeForViewTypeMap() const;

		const std::unordered_map<rtti::TypeInfo, SequenceEventTrackEditEventMemFun>& getEditEventHandlerMap() const;

		const std::unordered_map<rttr::type, SequenceEventTrackPasteEventMemFun>& getPasteEventMap() const;
	protected:
		/**
		 * registers all objects that need a specific way of construction
		 * @param factory the factory to register the object creators with
		 */
		void registerObjectCreators(rtti::Factory& factory) override;

		/**
		 * initializes service
		 * @param errorState contains any errors
		 * @return returns true on successful initialization
		 */
		bool init(nap::utility::ErrorState& errorState) override;

		/**
		 * updates any outputs and editors
		 * @param deltaTime deltaTime
		 */
		void update(double deltaTime) override;
	private:
		// map of segment view types
		SequenceEventTrackSegmentViewMap mEventSegmentViews;

		// map of view factory functions
		SequenceTrackViewFactoryMap mTrackViewFactoryMap;

		// map of segment edit event handlers
		std::unordered_map<rttr::type, SequenceEventTrackEditEventMemFun> mEditEventHandlerMap;

		// map of segment paste handlers
		std::unordered_map<rttr::type, SequenceEventTrackPasteEventMemFun> mPastEventMap;

		// list of event types
		std::vector<rtti::TypeInfo> mEventTypes;

		// which view type belongs to which track type
		SequenceTrackTypeForViewTypeMap mTrackViewTypeMap;
	};
}
