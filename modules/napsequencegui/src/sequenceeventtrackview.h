#pragma once

#include "sequencetrackview.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	class SequenceEventTrackView : public SequenceTrackView
	{
	public:
		SequenceEventTrackView(SequenceEditorGUIView& view, SequenceEditorGUIState& state);

		virtual void drawTrack(const SequenceTrack& track) override;

		virtual void handlePopups() override;
	protected:
		/**
		 * handlerInsertEventSegmentPopup
		 * handles insert event segment popup
		 */
		void handleInsertEventSegmentPopup();

		/**
		 * handleEditEventSegmentPopup
		 * handles event segment popup
		 */
		template<typename T>
		void handleEditEventSegmentPopup();

		/**
		 * drawEventTrack
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
		 * drawSegmentHandler
		 * draws segment handler
		 * @param track reference to track
		 * @param segment reference to segment
		 * @param trackTopLeft tracks topleft position
		 * @param segmentX segment x position
		 * @param segmentWidth width of segment
		 * @param drawList pointer to window drawlist
		 */
		void drawSegmentHandler(const SequenceTrack& track, const SequenceTrackSegment& segment, const ImVec2 &trackTopLeft, const float segmentX, const float segmentWidth, ImDrawList* drawList);

		void handleDeleteSegmentPopup();
	};

	template<>
	void SequenceEventTrackView::handleEditEventSegmentPopup<float>();

	template<>
	void SequenceEventTrackView::handleEditEventSegmentPopup<int>();

	template<>
	void SequenceEventTrackView::handleEditEventSegmentPopup<std::string>();

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
}