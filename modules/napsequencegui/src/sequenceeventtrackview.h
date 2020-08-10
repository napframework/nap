#pragma once

#include "sequencetrackview.h"
#include "sequencecontrollerevent.h"
#include "sequencetracksegment.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * SequenceEventTrackView is a view for event tracks
	 */
	class SequenceEventTrackView : public SequenceTrackView
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
		virtual void handlePopups() override;
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
		void handleDeleteSegmentPopup();

		/**
		 * creates an event edit action of specified type
		 * @tparam T type of event action
		 * @param segment base class of event segment
		 * @param trackID track id
		 * @param segmentID segment id
		 */
		template<typename T>
		void createEditAction(const SequenceTrackSegmentEventBase* segment, const std::string& trackID, const std::string& segmentID);

		// static map of popup function pointers
		static std::unordered_map<rttr::type, void(SequenceEventTrackView::*)()> sHandlePopupsMap;

		// static map of edit action function pointers
		static std::unordered_map<rttr::type, void(SequenceEventTrackView::*)(const SequenceTrackSegmentEventBase*, const std::string&, const std::string&)> sEditActionMap;
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
	void SequenceEventTrackView::handleEditEventSegmentPopup()
	{
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
				auto* action = mState.mAction->getDerived<SequenceGUIActions::EditingEventSegment<T>>();

				ImGui::SetWindowPos(action->mWindowPos);

				static std::unordered_map<rttr::type, void(*)(SequenceGUIActions::Action&)> s_handle_map{
						{ RTTI_OF(std::string), [](SequenceGUIActions::Action& action)
						{
							 auto* edit_action = action.getDerived<SequenceGUIActions::EditingEventSegment<std::string>>();
							 std::string& message = static_cast<std::string&>(edit_action->mValue);

							 char buffer[256];
							 strcpy(buffer, message.c_str());

							 if (ImGui::InputText("message", buffer, 256))
							 {
								 message = std::string(buffer);
							 }
						}},
						{ RTTI_OF(int), [](SequenceGUIActions::Action& action)
						{
						  	auto* edit_action = action.getDerived<SequenceGUIActions::EditingEventSegment<int>>();
						  	int& value = static_cast<int&>(edit_action->mValue);

						  	ImGui::InputInt("Value", &value);
						}},
						{ RTTI_OF(float), [](SequenceGUIActions::Action& action)
						{
						  	auto* editAction = action.getDerived<SequenceGUIActions::EditingEventSegment<float>>();
						  	float& value = static_cast<float&>(editAction->mValue);

						  	ImGui::InputFloat("Value", &value);
						}},
						{ RTTI_OF(glm::vec2), [](SequenceGUIActions::Action& action)
						{
							auto* edit_action = action.getDerived<SequenceGUIActions::EditingEventSegment<glm::vec2>>();
						  	glm::vec2& value = static_cast<glm::vec2&>(edit_action->mValue);

							ImGui::InputFloat2("Value", &value.x);
						}},
						{ RTTI_OF(glm::vec3), [](SequenceGUIActions::Action& action)
						{
							auto* edit_action = action.getDerived<SequenceGUIActions::EditingEventSegment<glm::vec3>>();
						  	glm::vec3& value = static_cast<glm::vec3&>(edit_action->mValue);

							ImGui::InputFloat3("Value", &value.x);
						}},
					};

				auto it = s_handle_map.find(RTTI_OF(T));
				assert(it!= s_handle_map.end()); // type not found
				if( it != s_handle_map.end())
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
	}


	template<typename T>
	void SequenceEventTrackView::createEditAction(const SequenceTrackSegmentEventBase* segment, const std::string& trackID, const std::string& segmentID)
	{
		const SequenceTrackSegmentEvent<T> *event = static_cast<const SequenceTrackSegmentEvent<T>*>(segment);
		mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::OpenEditEventSegmentPopup<T>>(trackID,segmentID,ImGui::GetWindowPos(), event->mValue, segment->mStartTime);
	}
}
