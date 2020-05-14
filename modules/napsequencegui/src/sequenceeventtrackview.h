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
			OpenEditEventSegmentPopup(std::string trackID, std::string segmentID, ImVec2 windowPos, T value)
				: mTrackID(trackID), mSegmentID(segmentID), mWindowPos(windowPos), mValue(value) {}

			std::string mTrackID;
			std::string mSegmentID;
			ImVec2 mWindowPos;
			T mValue;
		};

		template<typename T>
		class EditingEventSegment : public Action
		{
			RTTI_ENABLE(Action)
		public:
			EditingEventSegment(std::string trackID, std::string segmentID, ImVec2 windowPos, T value)
				: mTrackID(trackID), mSegmentID(segmentID), mWindowPos(windowPos), mValue(value) {}

			std::string mTrackID;
			std::string mSegmentID;
			T mValue;
			ImVec2 mWindowPos;
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
			mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::EditingEventSegment<T>>(action->mTrackID, action->mSegmentID, action->mWindowPos, action->mValue);
		}

		// handle insert segment popup
		if (mState.mAction->isAction<SequenceGUIActions::EditingEventSegment<T>>())
		{
			if (ImGui::BeginPopup("Edit Event"))
			{
				auto* action = mState.mAction->getDerived<SequenceGUIActions::EditingEventSegment<T>>();

				ImGui::SetWindowPos(action->mWindowPos);

				static std::unordered_map<rttr::type, void(*)(SequenceGUIActions::Action&)> sHandleMap
					{
						{ RTTI_OF(std::string), [](SequenceGUIActions::Action& action)
						{
							 auto* editAction = action.getDerived<SequenceGUIActions::EditingEventSegment<std::string>>();
							 std::string& message = static_cast<std::string&>(editAction->mValue);

							 int n = message.length();
							 char buffer[256];
							 strcpy(buffer, message.c_str());

							 if (ImGui::InputText("message", buffer, 256))
							 {
								 message = std::string(buffer);
							 }
						}},
						{ RTTI_OF(int), [](SequenceGUIActions::Action& action)
						{
						  	auto* editAction = action.getDerived<SequenceGUIActions::EditingEventSegment<int>>();
						  	int& value = static_cast<int&>(editAction->mValue);

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
							auto* editAction = action.getDerived<SequenceGUIActions::EditingEventSegment<glm::vec2>>();
						  	glm::vec2& value = static_cast<glm::vec2&>(editAction->mValue);

							ImGui::InputFloat2("Value", &value.x);
						}},
						{ RTTI_OF(glm::vec3), [](SequenceGUIActions::Action& action)
						{
							auto* editAction = action.getDerived<SequenceGUIActions::EditingEventSegment<glm::vec3>>();
						  	glm::vec3& value = static_cast<glm::vec3&>(editAction->mValue);

							ImGui::InputFloat3("Value", &value.x);
						}},
					};

				auto it = sHandleMap.find(RTTI_OF(T));
				assert(it!=sHandleMap.end()); // type not found
				if( it != sHandleMap.end())
				{
					it->second(*action);
				}

				if (ImGui::Button("Done"))
				{
					auto& eventController = getEditor().getController<SequenceControllerEvent>();
					eventController.editEventSegment<T>(action->mTrackID, action->mSegmentID, action->mValue);
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
		mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::OpenEditEventSegmentPopup<T>>(trackID,segmentID,ImGui::GetWindowPos(), event->mValue);
	}
}