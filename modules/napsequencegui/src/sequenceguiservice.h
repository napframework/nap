/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/service.h>
#include <entity.h>
#include <nap/datetime.h>
#include <rtti/factory.h>
#include <imguiservice.h>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceEventTrackSegmentViewBase;
	class SequenceEventTrackView;
	class SequenceTrackSegmentEventBase;
	class SequenceTrackView;
	class SequenceGUIService;
	class SequenceEditorGUIView;
	class SequenceEditorGUIState;

	// shortcuts
	using SequenceEventTrackSegmentViewFactoryFunc	= std::function<std::unique_ptr<SequenceEventTrackSegmentViewBase>()>;
	using SequenceEventTrackSegmentViewFactoryMap 	= std::unordered_map<rtti::TypeInfo, SequenceEventTrackSegmentViewFactoryFunc>;
	using SequenceTrackViewFactoryFunc 				= std::function<std::unique_ptr<SequenceTrackView>(SequenceGUIService&, SequenceEditorGUIView&, SequenceEditorGUIState&)>;
	using SequenceTrackViewFactoryMap 				= std::unordered_map<rtti::TypeInfo, SequenceTrackViewFactoryFunc>;
	using SequenceTrackTypeForViewTypeMap			= std::unordered_map<rtti::TypeInfo, rtti::TypeInfo>;
	using SequenceEventTrackPasteFunc 				= std::function<void(SequenceEventTrackView&, const std::string&, const SequenceTrackSegmentEventBase&, double)>;
	using SequenceEventTrackEditFunc 				= std::function<void(SequenceEventTrackView&)>;

	namespace icon
	{
		namespace sequencer
		{
			inline constexpr const char* play		= "seq_play.png";
			inline constexpr const char* stop		= "seq_stop.png";
			inline constexpr const char* rewind		= "seq_replay.png";
			inline constexpr const char* up			= "seq_up-arrow.png";
			inline constexpr const char* down		= "seq_down-arrow.png";
			inline constexpr const char* pause		= "seq_pause.png";
			inline constexpr const char* unpause	= "seq_unpause.png";
			inline constexpr const char* plus		= "seq_plus.png";
			inline constexpr const char* minus		= "seq_minus.png";
		}
	}

	/**
	 * The SequenceGUIService is responsible for registering track, segment & popup views and supplying the GUI the
	 * necessary factory methods to dynamically create the views and handlers for all registered types.
	 * When adding a new track type, a new segment type or a new event type with new gui views outside the SequenceGUI
	 * module you need to register them by calling registerTrackTypeForView, registerTrackViewFactory and/or
	 * registerEventView on the SequenceGUIService after the service is initialized but before any SequenceEditorGUI is
	 * created. A good place to do this from is from your own Service that will always initialize itself before any
	 * SequenceEditorGUI Resource.
	 */
	class NAPAPI SequenceGUIService final : public Service
	{
		friend class SequencePlayerOutput;

		RTTI_ENABLE(Service)
	public:

		/**
		 * Colors palette used by all sequencer Gui
		 */
		struct Colors
		{
			/**
			 * Initialize palette against configurable ImGUI color palette
			 * @param palette ImGUI color palette
			 */
			void init(const IMGuiColorPalette& palette);

			ImU32 mHigh = 0;		///< Special Highlight Color
			ImU32 mDark = 0;		///< Dark background Color
			ImU32 mBack = 0;		///< Background color
			ImU32 mFro1 = 0;		///< Darker Foreground
			ImU32 mFro2 = 0;		///< Dark Foreground
			ImU32 mFro3 = 0;		///< Light Foreground

			ImU32 mCurveColors[4] = { 4285098440, 4278255360, 4294901760, 4278255615 };
		};

		SequenceGUIService(ServiceConfiguration* configuration);
		~SequenceGUIService() override;

		/**
		 * registers object creator method that can be passed on to the rtti factory
		 * @param objectCreator unique pointer to method
		 */
		static bool registerObjectCreator(std::unique_ptr<rtti::IObjectCreator>(*objectCreator)(SequenceGUIService*));

		/**
		 * call this method to register a custom view for a custom event type
		 * T is the value type of the event (SequenceEvent<T>)
		 * @tparam T value to of the event
		 * @return true on successful registration
		 */
		template<typename T>
		bool registerEventView();

		/**
		 * registers a factory function for the creation of a certain track type,
		 * @param trackType the track type for which to register the factory function
		 * @param factory the factory function
		 * @return true on success
		 */
		bool registerTrackViewFactory(rtti::TypeInfo trackType, SequenceTrackViewFactoryFunc factory);

		/**
		 * registers the track view type that belongs to a track type
		 * @param trackType the track type
		 * @param viewType the view type
		 * @return true on success
		 */
		bool registerTrackTypeForView(rtti::TypeInfo trackType, rtti::TypeInfo viewType);

		/**
		 * invoke this method to create a track view for a certain view type, asserts when view type is not found
		 * @param viewType the viewType to create
		 * @param view reference to editor view creating the view
		 * @param state reference to gui state
		 * @return unique ptr to created track view
		 */
		std::unique_ptr<SequenceTrackView> invokeTrackViewFactory(rtti::TypeInfo viewType, SequenceEditorGUIView& view, SequenceEditorGUIState& state);

		/**
		 * invoke this method to create a event segment view for a certain event type, assert when event type is not found
		 * @param eventType type info of the event type
		 * @return unique ptr to create segment view
		 */
		std::unique_ptr<SequenceEventTrackSegmentViewBase> invokeEventTrackSegmentViewFactory(rtti::TypeInfo eventType);

		/**
		 * returns track view type for corresponding track type, asserts when track type not found
		 * @param trackType type info of track type
		 * @return corresponding track view type
		 */
		rtti::TypeInfo getViewTypeForTrackType(rtti::TypeInfo trackType) const;

		/**
		 * returns all registered event segment types (SequenceTrackSegmentEvent<T>)
		 * @return the vector containing type info of all registered event segment types
		 */
		const std::vector<rtti::TypeInfo>& getRegisteredSegmentEventTypes() const;

		/**
		 * call this method to invoke the edit event handler for a specific event action
		 * @param actionType type info of the action (f.e. EditingEventSegment<T>)
		 * @param view reference to the view invoking the edit event handler
		 */
		void invokeEditEventHandler(rtti::TypeInfo actionType, SequenceEventTrackView& view) const;


		/**
		 * invoke this method to paste an event of a certain type
		 * @param eventType typeInfo of the event to paste
		 * @param view reference to the view invoking the paste event
		 * @param trackID track id of the track on which to paste the event
		 * @param eventBase reference to base of event
		 * @param time time at which to paste the event
		 */
		void invokePasteEvent(rtti::TypeInfo eventType, SequenceEventTrackView& view, const std::string& trackID, const SequenceTrackSegmentEventBase& eventBase, double time) const;

		/**
		 * returns a vector containing type info of all registered track types
		 * @return the vector
		 */
		std::vector<rtti::TypeInfo> getAllTrackTypes() const;

		/**
		 * returns a vector containing type info of all registered event actions (f.e. EditingEventSegment<T>)
		 * @return the vector
		 */
		std::vector<rtti::TypeInfo> getAllRegisteredEventActions() const;

		/**
		 * @return gui service
		 */
		nap::IMGuiService& getGui();

		/**
		 * @return gui service
		 */
		const nap::IMGuiService& getGui() const;

		/**
		 * Returns the sequencer GUI color palette 
		 * @return sequencer GUI color palette
		 */
		const Colors& getColors() const { return mColors; }
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
         * Override this function to register service dependencies
         * A service that depends on another service is initialized after all it's associated dependencies
         * This will ensure correct order of initialization, update calls and shutdown of all services
         * SequenceGUIService depends on SequenceService
         * @param dependencies rtti information of the services this service depends on
         */
		virtual void getDependentServices(std::vector<rtti::TypeInfo>& dependencies);

	private:
		// map of segment view types
		SequenceEventTrackSegmentViewFactoryMap mEventSegmentViewFactoryMap;

		// map of view factory functions
		SequenceTrackViewFactoryMap mTrackViewFactoryMap;

		// map of segment edit event handlers
		std::unordered_map<rtti::TypeInfo, SequenceEventTrackEditFunc> mEditEventHandlerMap;

		// map of segment paste handlers
		std::unordered_map<rtti::TypeInfo, SequenceEventTrackPasteFunc> mPasteEventMap;

		// list of event types
		std::vector<rtti::TypeInfo> mSegmentEventTypes;

		// which view type belongs to which track type
		SequenceTrackTypeForViewTypeMap mTrackViewTypeMap;

		// Link to the gui service
		IMGuiService* mGuiService = nullptr;

		// Colors
		Colors mColors;
	};
}
