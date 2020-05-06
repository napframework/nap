#pragma once

#include "sequencetrackview.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

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

		class OpenEditEventSegmentPopup : public Action 
		{ 
			RTTI_ENABLE(Action) 
		public:
			OpenEditEventSegmentPopup(std::string trackID, std::string segmentID, ImVec2 windowPos, std::string message)
				: mTrackID(trackID), mSegmentID(segmentID), mWindowPos(windowPos), mMessage(message) {}

			std::string mTrackID;
			std::string mSegmentID;
			ImVec2 mWindowPos;
			std::string mMessage;
		};

		class EditingEventSegment : public Action 
		{
			RTTI_ENABLE(Action)
		public:
			EditingEventSegment(std::string trackID, std::string segmentID, ImVec2 windowPos)
				: mTrackID(trackID), mSegmentID(segmentID), mWindowPos(windowPos) {}

			std::string mTrackID;
			std::string mSegmentID;
			std::string mMessage = "hello world";
			ImVec2 mWindowPos;
		};
	}

	class SequenceEventTrackView : public SequenceTrackView
	{
	public:
		SequenceEventTrackView(SequenceEditorGUIView& view);

		virtual void drawTrack(const SequenceTrack& track, SequenceEditorGUIState& state) override;

		virtual void handlePopups(SequenceEditorGUIState& state) override;
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
}