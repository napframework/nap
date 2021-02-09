/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "sequencetrackview.h"
#include "sequencecontrollerevent.h"
#include "sequencetracksegment.h"
#include "sequenceeditorguiclipboard.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceEventTrackView;

	/**
	 * Base class for an event segment view
	 * This base class is used by the track view to draw and handle segment views of different event types
	 * When you want to add your own event type, you can extend the derived class SequenceEventTrackSegmentView<T> where T is the value type of your own event
	 */
	class NAPAPI SequenceEventTrackSegmentViewBase
	{
		RTTI_ENABLE()
	public:
		/**
		 * Constructor
		 */
		SequenceEventTrackSegmentViewBase(){ }

		/**
		 * Deconstructor
		 */
		virtual ~SequenceEventTrackSegmentViewBase() = default;

		/**
		 * Extend this method to implement the way editing this event type must be handle in the GUI
		 * For examples, see template specialisations in SequenceEventTrackView.cpp
		 * @param action the incoming action from the gui, contains information about the track time and segment. Segment can be assumed to be of type SequenceTrackSegmentEvent<T>
		 */
		virtual void handleEditPopupContent(SequenceGUIActions::Action& action) = 0;

		/**
		 * Extend this method to specify a way to draw this event type
		 * For examples, see template specialisations in SequenceEventTrackView.cpp
		 * @param segment reference to segment
		 * @param drawList pointer to ImGui drawlist
		 * @param topLeft top left position
		 * @param x x position of segment on track
		 */
		virtual void drawEvent(const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, const float x) = 0;

		/**
		 * Extend this method to specify the way the controller needs to be called to add your custom event type
		 * Generally, this method doesn't need specialisation
		 * @param controller reference to controller
		 * @param trackID id of event track
		 * @param time time at which to insert custom event
		 */
		virtual void insertSegment(SequenceControllerEvent& controller, const std::string& trackID, double time) = 0;

		/**
		 * Extend this method to specify the way an edit action for this event segment needs to be created
		 * Generally, this method doesn't need specialisation
		 * @param segment const pointer to segment
		 * @param trackID the track id
		 * @param segmentID the segment id
		 * @return unique pointer to created action, cannot be nullptr
		 */
		virtual std::unique_ptr<SequenceGUIActions::Action> createEditAction(const SequenceTrackSegmentEventBase* segment, const std::string& trackID, const std::string& segmentID) = 0;
	protected:
	};

	/**
	 * The SequenceEventTrackSegmentView<T> is responsible for drawing and handling the GUI for event types of type T
	 * The track view looks up the appropriate view for each event type exists on the track.
	 * You can register new views for new type of events from outside, enabling to write your own views and add your own event types relatively simple
	 * Extend this class if you want a view for your own event type
	 * Override the methods "handleEditPopupContent" and "drawEvent" when implementing your custom view for your custom event
	 * For examples. Take a look at the template specialisations in SequenceEventTrackView.cpp
	 */
	template<typename T>
	class NAPAPI SequenceEventTrackSegmentView : public SequenceEventTrackSegmentViewBase
	{
		RTTI_ENABLE(SequenceEventTrackSegmentViewBase)
	public:
		/**
		 * Constructor
		 */
		SequenceEventTrackSegmentView() : SequenceEventTrackSegmentViewBase(){}

		/**
		 * This method needs specialization in order to implement the way popups are handled when editing this segment
		 * For examples, see template specialisations in SequenceEventTrackView.cpp
		 * @param action the incoming action from the gui, contains information about the track time and segment. Segment can be assumed to be of type SequenceTrackSegmentEvent<T>
		 */
		void handleEditPopupContent(SequenceGUIActions::Action& action) override;

		/**
		 * Specialise this method to specify a way to draw this event type
		 * For examples, see template specialisations in SequenceEventTrackView.cpp
		 * @param segment reference to segment
		 * @param drawList pointer to ImGui drawlist
		 * @param topLeft top left position
		 * @param x x position of segment on track
		 */
		void drawEvent(const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, const float x) override;

		/**
		 * Specialise this method to specify the way the controller needs to be called to add your custom event type
		 * Generally, this method doesn't need specialisation
		 * @param controller reference to controller
		 * @param trackID id of event track
		 * @param time time at which to insert custom event
		 */
		void insertSegment(SequenceControllerEvent& controller, const std::string& trackID, double time) override;

		/**
		 * Specialise this method to specify the way an edit action for this event segment needs to be created
		 * Generally, this method doesn't need specialisation
		 * @param segment const pointer to segment
		 * @param trackID the track id
		 * @param segmentID the segment id
		 * @return unique pointer to created action, cannot be nullptr
		 */
		std::unique_ptr<SequenceGUIActions::Action> createEditAction(const SequenceTrackSegmentEventBase* segment, const std::string& trackID, const std::string& segmentID) override;
	protected:
	};


	/**
	 * SequenceEventTrackView is a view for event tracks
	 */
	class NAPAPI SequenceEventTrackView : public SequenceTrackView
	{
		friend class SequenceEventTrackSegmentViewBase;
		RTTI_ENABLE(SequenceTrackView)
	public:
		/**
		 * Constructor
		 * @param view reference to editor view
		 * @param state reference to editor state
		 */
		SequenceEventTrackView(SequenceEditorGUIView& view, SequenceEditorGUIState& state);

		/**
		 * call this static method to register you a custom view for a custom event type
		 * T is the value type of the event ( SequenceEvent<T> )
		 * @tparam T value to of the event
		 * @return true when called
		 */
		template<typename T>
		static bool registerEventView();
	protected:
		/**
		 * shows inspector content
		 * @param track reference to track
		 */
		virtual void showInspectorContent(const SequenceTrack& track) override ;

		/**
		 * shows track contents
		 * @param track reference to track
		 * @param trackTopLeft orientation
		 */
		virtual void showTrackContent(const SequenceTrack& track, const ImVec2& trackTopLeft) override;

		/**
		 * handles insert event segment popup
		 */
		void handleInsertEventSegmentPopup();

		/**
		 * handles event segment popup
		 */
		template<typename T>
		void handleEditEventSegmentPopup();

		/**
		 * handles pasting of clipboard content to event segment
		 * @tparam T the event segment type
		 * @param trackID the track id of where to paste the new event
		 * @param time the time where to insert copied event segment
		 */
		void pasteEventsFromClipboard(const std::string& trackID, double time);

		/**
		 * Paste events of type T. Base event is base class of event of type T
		 * @tparam T type of event
		 * @param trackID track on which to paste event
		 * @param baseEvent reference to base class of event to paste
		 * @param time time in seconds
		 */
		template<typename T>
		void pasteEvent(const std::string& trackID, const SequenceTrackSegmentEventBase& baseEvent, double time);

		/**
		 * draws segment handler
		 * @param track reference to track
		 * @param segment reference to segment
		 * @param trackTopLeft tracks topleft position
		 * @param segmentX segment x position
		 * @param segmentWidth width of segment
		 * @param drawList pointer to window drawlist
		 */
		void drawSegmentHandler(const SequenceTrack& track, const SequenceTrackSegment& segment, const ImVec2 &trackTopLeft, const float segmentX, const float segmentWidth, ImDrawList* drawList);

		/**
		 * handles delete segment popup
		 */
		void handleEditSegmentValuePopup();

		/**
		 * update the segment in the clipboard
		 * @param trackID the track id of the track containing the segment
		 * @param segmentID the segment id
		 */
		void updateSegmentInClipboard(const std::string& trackID, const std::string& segmentID);
	private:
		// map of segment view types
		static std::unordered_map<rttr::type, std::unique_ptr<SequenceEventTrackSegmentViewBase>>& getSegmentViews();

		// map of segment edit event handlers
		static std::unordered_map<rttr::type, void (SequenceEventTrackView::*)()>& getEditEventHandlers();

		// map of segment paste handlers
		static std::unordered_map<rttr::type, void (SequenceEventTrackView::*)(const std::string&, const SequenceTrackSegmentEventBase&, double)>& getPasteEventMap();

		// list of event types
		static std::vector<rttr::type>& getEventTypesVector();
	};


	//////////////////////////////////////////////////////////////////////////
	// Event Actions
	//////////////////////////////////////////////////////////////////////////

	namespace SequenceGUIActions
	{
		class OpenInsertEventSegmentPopup : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			OpenInsertEventSegmentPopup(const std::string& trackID, double time)
				: TrackAction(trackID), mTime(time) {}

			double mTime;
		};

		class InsertingEventSegment : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			InsertingEventSegment(const std::string& trackID, double time)
				: TrackAction(trackID), mTime(time) {}

			double mTime;
			std::string mMessage = "hello world";
		};

		template<typename T>
		class OpenEditEventSegmentPopup : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			OpenEditEventSegmentPopup(const std::string& trackID, const std::string& segmentID, ImVec2 windowPos, T value, double startTime)
				: TrackAction(trackID), mSegmentID(segmentID), mWindowPos(windowPos), mValue(value), mStartTime(startTime) {}

			std::string mSegmentID;
			ImVec2 mWindowPos;
			T mValue;
			double mStartTime;
		};

		template<typename T>
		class EditingEventSegment : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			EditingEventSegment(const std::string& trackID, const std::string& segmentID, ImVec2 windowPos, T value, double startTime)
				: TrackAction(trackID), mSegmentID(segmentID), mWindowPos(windowPos), mValue(value), mStartTime(startTime) {}

			std::string mSegmentID;
            ImVec2 mWindowPos;
			T mValue;			
			double mStartTime;
		};
	}

	//////////////////////////////////////////////////////////////////////////
	// Event Clipboards
	//////////////////////////////////////////////////////////////////////////

	namespace SequenceGUIClipboards
	{
		class EventSegmentClipboard :
			public Clipboard
		{
			RTTI_ENABLE(Clipboard)
		public:
			EventSegmentClipboard(const rttr::type& type, const std::string& sequenceName) : Clipboard(type), mSequenceName(sequenceName){};

			const std::string& getSequenceName() const { return mSequenceName; }
		private:
			std::string mSequenceName;
		};
	}

	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	void SequenceEventTrackView::handleEditEventSegmentPopup()
	{
		if (mState.mAction->isAction<SequenceGUIActions::OpenEditEventSegmentPopup<T>>())
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Edit Event");

			auto* action = mState.mAction->getDerived<SequenceGUIActions::OpenEditEventSegmentPopup<T>>();
			ImGui::SetNextWindowPos(action->mWindowPos);

			mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::EditingEventSegment<T>>(action->mTrackID, action->mSegmentID, action->mWindowPos, action->mValue, action->mStartTime);
		}

		// handle insert segment popup
		if (mState.mAction->isAction<SequenceGUIActions::EditingEventSegment<T>>())
		{
			auto* action = mState.mAction->getDerived<SequenceGUIActions::EditingEventSegment<T>>();

			if (ImGui::BeginPopup("Edit Event"))
			{
				// draw the registered popup content for this event
				auto& segment_views = getSegmentViews();
				auto it = segment_views.find(RTTI_OF(SequenceTrackSegmentEvent<T>));
				assert(it!= segment_views.end()); // type not found
				if( it != segment_views.end())
				{
					it->second->handleEditPopupContent(*action);
				}

				// time
				int time_milseconds = (int) ( ( action->mStartTime ) * 100.0 ) % 100;
				int time_seconds = (int) ( action->mStartTime ) % 60;
				int time_minutes = (int) ( action->mStartTime ) / 60;

				bool edit_time = false;

				ImGui::Separator();

				ImGui::PushItemWidth(100.0f);

				int time_array[3] =
					{
						time_minutes,
						time_seconds,
						time_milseconds
					};

				edit_time = ImGui::InputInt3("Time (mm:ss:ms)", &time_array[0]);
				time_array[0] = math::clamp<int>(time_array[0], 0, 99999);
				time_array[1] = math::clamp<int>(time_array[1], 0, 59);
				time_array[2] = math::clamp<int>(time_array[2], 0, 99);

				ImGui::PopItemWidth();

				ImGui::Separator();

				if( edit_time )
				{
					auto& event_controller = getEditor().getController<SequenceControllerEvent>();
					double new_time = ( ( (double) time_array[2] )  / 100.0 ) + (double) time_array[1] + ( (double) time_array[0] * 60.0 );
					double time = event_controller.segmentEventStartTimeChange(action->mTrackID, action->mSegmentID, new_time);
					updateSegmentInClipboard(action->mTrackID, action->mSegmentID);
					action->mStartTime = time;
					mState.mDirty = true;
				}

				if (ImGui::Button("Done"))
				{
					auto& event_controller = getEditor().getController<SequenceControllerEvent>();
					event_controller.editEventSegment<T>(action->mTrackID, action->mSegmentID, action->mValue);
					updateSegmentInClipboard(action->mTrackID, action->mSegmentID);
					mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::None>();
                    ImGui::CloseCurrentPopup();
				}

				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
					mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::None>();
				}

				action->mWindowPos = ImGui::GetWindowPos();

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::None>();
			}
		}
	}


	template<typename T>
	bool SequenceEventTrackView::registerEventView()
	{
		// register type of view
		auto& types = getEventTypesVector();
		assert(std::find(types.begin(), types.begin() + types.size(), RTTI_OF(SequenceTrackSegmentEvent<T>)) == types.end()); // type already added
		types.emplace_back(RTTI_OF(SequenceTrackSegmentEvent<T>));

		// register view
		auto& segment_views = getSegmentViews();

		auto segment_it = segment_views.find(RTTI_OF(SequenceTrackSegmentEvent<T>));
		assert(segment_it==segment_views.end()); // type already registered
		if(segment_it==segment_views.end())
		{
			segment_views.emplace(RTTI_OF(SequenceTrackSegmentEvent<T>), std::make_unique<SequenceEventTrackSegmentView<T>>());
		}

		// register popup action handler
		auto& handle_edit_events = getEditEventHandlers();

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
		auto& handler_paste_events = getPasteEventMap();

		auto paste_it = handler_paste_events.find(RTTI_OF(SequenceTrackSegmentEvent<T>));
		assert(paste_it == handler_paste_events.end()); // type already registered
		if(paste_it == handler_paste_events.end())
		{
			handler_paste_events.emplace(RTTI_OF(SequenceTrackSegmentEvent<T>), &SequenceEventTrackView::pasteEvent<SequenceTrackSegmentEvent<T>> );
		}

		return true;
	}


	template<typename T>
	void SequenceEventTrackView::pasteEvent(const std::string& trackID, const SequenceTrackSegmentEventBase& baseEvent, double time)
	{
		// obtain controller
		auto& controller = getEditor().getController<SequenceControllerEvent>();

		// insert new segment
		const auto* new_segment = static_cast<const T*>(controller.insertEventSegment<T>(trackID, baseEvent.mStartTime + time));

		// upcast de-serialized event
		const auto* event_upcast = rtti_cast<const T>(&baseEvent);

		// cannot be null
		assert(event_upcast != nullptr);

		// copy values from deserialized event segment
		controller.editEventSegment(trackID, new_segment->mID, event_upcast->mValue);
	}


	template<typename T>
	void SequenceEventTrackSegmentView<T>::insertSegment(SequenceControllerEvent& controller, const std::string& trackID, double time)
	{
		controller.insertEventSegment<SequenceTrackSegmentEvent<T>>(trackID, time);
	}


	template<typename T>
	std::unique_ptr<SequenceGUIActions::Action> SequenceEventTrackSegmentView<T>::createEditAction(const SequenceTrackSegmentEventBase* segment, const std::string& trackID, const std::string& segmentID)
	{
		const auto *event = static_cast<const SequenceTrackSegmentEvent<T>*>(segment);
		return SequenceGUIActions::createAction<SequenceGUIActions::OpenEditEventSegmentPopup<T>>(trackID,segmentID,ImGui::GetWindowPos(), event->mValue, segment->mStartTime);
	}
}
