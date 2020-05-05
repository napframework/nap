#pragma once

#include "sequencetrackview.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	namespace SequenceGUIActions
	{
		struct OpenInsertEventSegmentPopup : SequenceGUIAction { RTTI_ENABLE(SequenceGUIAction) };
		struct InsertingEventSegment : SequenceGUIAction { RTTI_ENABLE(SequenceGUIAction) };
		struct OpenEditEventSegmentPopup : SequenceGUIAction { RTTI_ENABLE(SequenceGUIAction) };
		struct EditingEventSegment : SequenceGUIAction { RTTI_ENABLE(SequenceGUIAction) };
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

	/**
	* Data needed for handling insertion of event segments
	*/
	class SequenceGUIInsertEventSegment : public SequenceGUIActionData
	{
	public:
		SequenceGUIInsertEventSegment(std::string id, double aTime)
			: mTrackID(id), mTime(aTime) {}

		std::string mTrackID;
		double mTime;
		std::string mEventMessage = "Hello world";
	};

	/**
	* Data needed for editing event segments
	*/
	class SequenceGUIEditEventSegment : public SequenceGUIActionData
	{
	public:
		SequenceGUIEditEventSegment(std::string trackId, std::string segmentID, std::string message, ImVec2 windowPos)
			:
			mTrackID(trackId),
			mSegmentID(segmentID),
			mMessage(message),
			mWindowPos(windowPos) {}

		std::string mTrackID;
		std::string mSegmentID;
		std::string mMessage;
		ImVec2 mWindowPos;
	};
}