/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "sequencetrackview.h"
#include "sequencecontrollerevent.h"
#include "sequencetracksegment.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	class SequenceEventTrackView;

	using SequenceEventTrackViewHandlePopupFunc 		= bool(SequenceEventTrackView::*)();
	using SequenceEventTrackViewEditActionFunc 			= void(SequenceEventTrackView::*)(const SequenceTrackSegmentEventBase*, const std::string&, const std::string&);
	using SequenceEventTrackViewHandlePopupContentFunc 	= void(*)(SequenceGUIActions::Action&);
	using SequenceEventTrackViewDrawEventFunc			= void(*)(const SequenceTrackSegment& segment, ImDrawList*, const ImVec2&, const float);
	using SequenceEventTrackViewInsertSegmentFunc		= void(*)(SequenceControllerEvent&, std::string&, double);

	/**
	 * SequenceEventTrackView is a view for event tracks
	 */
	class NAPAPI SequenceEventTrackView : public SequenceTrackView
	{
	public:
		/**
		 * Constructor
		 * @param view reference to editor view
		 * @param state reference to editor state
		 */
		SequenceEventTrackView(SequenceEditorGUIView& view, SequenceEditorGUIState& state);

		/**
		 * handles popups
		 */
		virtual bool handlePopups() override;

		template<typename T>
		static void registerEventType(SequenceEventTrackViewHandlePopupFunc,
									  SequenceEventTrackViewEditActionFunc,
									  SequenceEventTrackViewHandlePopupContentFunc,
									  SequenceEventTrackViewDrawEventFunc,
									  SequenceEventTrackViewInsertSegmentFunc);

		template <typename T>
		static SequenceEventTrackViewHandlePopupFunc getHandlePopupFuncPtr()
		{
			return &SequenceEventTrackView::handleEditEventSegmentPopup<T>;
		}

		template <typename T>
		static SequenceEventTrackViewEditActionFunc getCreateEditActionFuncPtr()
		{
			return &SequenceEventTrackView::createEditAction<T>;
		}
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
		bool handleInsertEventSegmentPopup();

		/**
		 * handles event segment popup
		 */
		template<typename T>
		bool NAPAPI handleEditEventSegmentPopup();

		/**
		 * draws event track
		 * @param track reference to track
		 * @param cursorPos imgui cursorposition
		 * @param marginBetweenTracks y margin between tracks
		 * @param sequencePlayer reference to sequence player
		 * @param deleteTrack set to true when delete track button is pressed
		 * @param deleteTrackID the id of track that needs to be deleted
		 */
		void drawEventTrack(const SequenceTrack &track, ImVec2 &cursorPos, const float marginBetweenTracks, const SequencePlayer &sequencePlayer, bool &deleteTrack, std::string &deleteTrackID);

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
		bool handleDeleteSegmentPopup();

		/**
		 * creates an event edit action of specified type
		 * @tparam T type of event action
		 * @param segment base class of event segment
		 * @param trackID track id
		 * @param segmentID segment id
		 */
		template<typename T>
		void createEditAction(const SequenceTrackSegmentEventBase* segment, const std::string& trackID, const std::string& segmentID);

	protected:
		static std::unordered_map<rttr::type, SequenceEventTrackViewHandlePopupContentFunc>& getHandlePopupContentMap();

		static std::unordered_map<rttr::type, SequenceEventTrackViewHandlePopupFunc>& getHandlePopupMap();

		// static map of edit action function pointers
		static std::unordered_map<rttr::type, SequenceEventTrackViewEditActionFunc>& getEditActionMap();

		static std::unordered_map<rttr::type, SequenceEventTrackViewDrawEventFunc>& getDrawEventsMap();

		static std::unordered_map<rttr::type, SequenceEventTrackViewInsertSegmentFunc>& SequenceEventTrackView::getInsertSegmentMap();

		static std::vector<rttr::type>& getEventTypesMap();
	};

	namespace SequenceGUIActions
	{
		class OpenInsertEventSegmentPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			OpenInsertEventSegmentPopup(std::string trackID, double time)
				: mTrackID(trackID), mTime(time) {}

			std::string mTrackID;
			double mTime;
		};

		class InsertingEventSegment : public Action
		{
			RTTI_ENABLE(Action)
		public:
			InsertingEventSegment(std::string trackID, double time)
				: mTrackID(trackID), mTime(time) {}

			std::string mTrackID;
			double mTime;
			std::string mMessage = "hello world";
		};

		template<typename T>
		class OpenEditEventSegmentPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			OpenEditEventSegmentPopup(std::string trackID, std::string segmentID, ImVec2 windowPos, T value, double startTime)
				: mTrackID(trackID), mSegmentID(segmentID), mWindowPos(windowPos), mValue(value), mStartTime(startTime) {}

			std::string mTrackID;
			std::string mSegmentID;
			ImVec2 mWindowPos;
			T mValue;
			double mStartTime;
		};

		template<typename T>
		class EditingEventSegment : public Action
		{
			RTTI_ENABLE(Action)
		public:
			EditingEventSegment(std::string trackID, std::string segmentID, ImVec2 windowPos, T value, double startTime)
				: mTrackID(trackID), mSegmentID(segmentID), mWindowPos(windowPos), mValue(value), mStartTime(startTime) {}

			std::string mTrackID;
			std::string mSegmentID;
            ImVec2 mWindowPos;
			T mValue;			
			double mStartTime;
		};
	}


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	bool SequenceEventTrackView::handleEditEventSegmentPopup()
	{
		bool handled = false;

		if (mState.mAction->isAction<SequenceGUIActions::OpenEditEventSegmentPopup<T>>())
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Edit Event");

			auto* action = mState.mAction->getDerived<SequenceGUIActions::OpenEditEventSegmentPopup<T>>();
			mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::EditingEventSegment<T>>(action->mTrackID, action->mSegmentID, action->mWindowPos, action->mValue, action->mStartTime);
		}

		// handle insert segment popup
		if (mState.mAction->isAction<SequenceGUIActions::EditingEventSegment<T>>())
		{
			if (ImGui::BeginPopup("Edit Event"))
			{
				handled = true;

				auto* action = mState.mAction->getDerived<SequenceGUIActions::EditingEventSegment<T>>();

				ImGui::SetWindowPos(action->mWindowPos);

				auto& handle_popups_map = getHandlePopupContentMap();
				auto it = handle_popups_map.find(RTTI_OF(T));
				assert(it!= handle_popups_map.end()); // type not found
				if( it != handle_popups_map.end())
				{
					it->second(*action);
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
					action->mStartTime = time;
					mState.mDirty = true;
				}

				if (ImGui::Button("Done"))
				{
					auto& event_controller = getEditor().getController<SequenceControllerEvent>();
					event_controller.editEventSegment<T>(action->mTrackID, action->mSegmentID, action->mValue);
					mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::None>();
				}

				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
					mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::None>();
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::None>();
			}
		}

		return handled;
	}


	template<typename T>
	void SequenceEventTrackView::createEditAction(const SequenceTrackSegmentEventBase* segment, const std::string& trackID, const std::string& segmentID)
	{
		const SequenceTrackSegmentEvent<T> *event = static_cast<const SequenceTrackSegmentEvent<T>*>(segment);
		mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::OpenEditEventSegmentPopup<T>>(trackID,segmentID,ImGui::GetWindowPos(), event->mValue, segment->mStartTime);
	}

	template<typename T>
	void SequenceEventTrackView::registerEventType(SequenceEventTrackViewHandlePopupFunc handlePopupFunc,
												   SequenceEventTrackViewEditActionFunc editActionFunc,
												   SequenceEventTrackViewHandlePopupContentFunc handlePopupContentFunc,
												   SequenceEventTrackViewDrawEventFunc drawEventFunc,
												   SequenceEventTrackViewInsertSegmentFunc insertSegmentFunc)
	{
		getEventTypesMap().emplace_back(RTTI_OF(SequenceTrackSegmentEvent<T>));
		getHandlePopupMap().insert({RTTI_OF(SequenceTrackSegmentEvent<T>), handlePopupFunc});
		getEditActionMap().insert({RTTI_OF(SequenceTrackSegmentEvent<T>), editActionFunc});
		getHandlePopupContentMap().insert({RTTI_OF(T), handlePopupContentFunc});
		getDrawEventsMap().insert({RTTI_OF(SequenceTrackSegmentEvent<T>), drawEventFunc});
		getInsertSegmentMap().insert({RTTI_OF(SequenceTrackSegmentEvent<T>), insertSegmentFunc});
	}
}
