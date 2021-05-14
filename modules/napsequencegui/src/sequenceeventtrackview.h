/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <utility>

#include "sequencetrackview.h"
#include "sequencecontrollerevent.h"
#include "sequencetracksegment.h"
#include "sequenceeditorguiclipboard.h"
#include "sequenceeventtrackview_guiactions.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceEventTrackView;
	class SequenceGUIService;

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
		SequenceEventTrackSegmentViewBase() = default;

		/**
		 * Deconstructor
		 */
		virtual ~SequenceEventTrackSegmentViewBase() = default;

		/**
		 * Extend this method to implement the way editing this event type must be handle in the GUI
		 * For examples, see template specializations in SequenceEventTrackView.cpp
		 * @param action the incoming action from the gui, contains information about the track time and segment. Segment can be assumed to be of type SequenceTrackSegmentEvent<T>
		 */
		virtual void handleEditPopupContent(SequenceGUIActions::Action& action) = 0;

		/**
		 * Extend this method to specify a way to draw this event type
		 * For examples, see template specializations in SequenceEventTrackView.cpp
		 * @param segment reference to segment
		 * @param drawList pointer to ImGui drawlist
		 * @param topLeft top left position
		 * @param x x position of segment on track
		 */
		virtual void drawEvent(const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, float x) = 0;

		/**
		 * Extend this method to specify the way the controller needs to be called to add your custom event type
		 * Generally, this method doesn't need specialization
		 * @param controller reference to controller
		 * @param trackID id of event track
		 * @param time time at which to insert custom event
		 */
		virtual void insertSegment(SequenceControllerEvent& controller, const std::string& trackID, double time) = 0;

		/**
		 * Extend this method to specify the way an edit action for this event segment needs to be created
		 * Generally, this method doesn't need specialization
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
	 * For examples. Take a look at the template specializations in SequenceEventTrackView.cpp
	 */
	template<typename T>
	class NAPAPI SequenceEventTrackSegmentView final : public SequenceEventTrackSegmentViewBase
	{
		RTTI_ENABLE(SequenceEventTrackSegmentViewBase)
	public:
		/**
		 * Constructor
		 */
		SequenceEventTrackSegmentView() : SequenceEventTrackSegmentViewBase(){}

		/**
		 * This method needs specialization in order to implement the way popups are handled when editing this segment
		 * For examples, see template specializations in SequenceEventTrackView.cpp
		 * @param action the incoming action from the gui, contains information about the track time and segment. Segment can be assumed to be of type SequenceTrackSegmentEvent<T>
		 */
		void handleEditPopupContent(SequenceGUIActions::Action& action) override;

		/**
		 * Specialize this method to specify a way to draw this event type
		 * For examples, see template specializations in SequenceEventTrackView.cpp
		 * @param segment reference to segment
		 * @param drawList pointer to ImGui drawlist
		 * @param topLeft top left position
		 * @param x x position of segment on track
		 */
		void drawEvent(const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, float x) override;

		/**
		 * Specialize this method to specify the way the controller needs to be called to add your custom event type
		 * Generally, this method doesn't need specialization
		 * @param controller reference to controller
		 * @param trackID id of event track
		 * @param time time at which to insert custom event
		 */
		void insertSegment(SequenceControllerEvent& controller, const std::string& trackID, double time) override;

		/**
		 * Specialize this method to specify the way an edit action for this event segment needs to be created
		 * Generally, this method doesn't need specialization
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
	class NAPAPI SequenceEventTrackView final : public SequenceTrackView
	{
		friend class SequenceGUIService;
		friend class SequenceEventTrackSegmentViewBase;

		RTTI_ENABLE(SequenceTrackView)
	public:
		/**
		 * Constructor
		 * @param service reference to gui service
		 * @param view reference to editor view
		 * @param state reference to editor state
		 */
		SequenceEventTrackView(SequenceGUIService& service, SequenceEditorGUIView& view, SequenceEditorGUIState& state);
	protected:
		/**
		 * shows inspector content
		 * @param track reference to track
		 */
		void showInspectorContent(const SequenceTrack& track) override ;

		/**
		 * shows track contents
		 * @param track reference to track
		 * @param trackTopLeft orientation
		 */
		void showTrackContent(const SequenceTrack& track, const ImVec2& trackTopLeft) override;

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
		void drawSegmentHandler(const SequenceTrack& track, const SequenceTrackSegment& segment, const ImVec2 &trackTopLeft, float segmentX, float segmentWidth, ImDrawList* drawList);

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

		/**
		 * handles assigning of new output id to track
		 */
		void handleAssignOutputIDToTrack();

		/**
		 * handles dragging of event segment
		 */
		void handleSegmentDrag();
	private:
		// for some reason we need to have this member variable declared in order the class to be compiled with VS2015
		std::unique_ptr<SequenceEventTrackSegmentViewBase> mSegmentViewNullPtr = nullptr;

		// map of segment views for different event views
		std::unordered_map<rtti::TypeInfo, std::unique_ptr<SequenceEventTrackSegmentViewBase>> mSegmentViews;
	};


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
			EventSegmentClipboard(const rttr::type& type, std::string  sequenceName) : Clipboard(type), mSequenceName(std::move(sequenceName)){};

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
				auto it = mSegmentViews.find(RTTI_OF(SequenceTrackSegmentEvent<T>));
				assert(it!= mSegmentViews.end()); // type not found
				it->second->handleEditPopupContent(*action);

				// time
				int time_milseconds = (int) ( ( action->mStartTime ) * 100.0 ) % 100;
				int time_seconds = (int) ( action->mStartTime ) % 60;
				int time_minutes = (int) ( action->mStartTime ) / 60;

				bool edit_time;

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
	void SequenceEventTrackView::pasteEvent(const std::string& trackID, const SequenceTrackSegmentEventBase& baseEvent, double time)
	{
		// obtain controller
		auto& controller = getEditor().getController<SequenceControllerEvent>();

		// insert new segment
		const auto* new_segment = rtti_cast<const T>(controller.insertEventSegment<T>(trackID, baseEvent.mStartTime + time));
		assert(new_segment!= nullptr); // cast failed

		// upcast de-serialized event
		const auto* event_upcast = rtti_cast<const T>(&baseEvent);
		assert(event_upcast!= nullptr); // cast failed

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
