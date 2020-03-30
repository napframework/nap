#pragma once

// internal includes
#include "sequenceeditor.h"

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <rtti/objectptr.h>
#include <imgui/imgui.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceEditorGUIView;
	class SequenceEditorView;

	/**
	 */
	class NAPAPI SequenceEditorGUI : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual bool init(utility::ErrorState& errorState);

		virtual void onDestroy();

		void draw();
	public:
		ResourcePtr<SequenceEditor> mSequenceEditor = nullptr;
	protected:
		std::unique_ptr<SequenceEditorGUIView> mView = nullptr;
	};


	enum SequenceGUIMouseActions
	{
		// ACTIONS
		DRAGGING_SEGMENT,
		INSERTING_SEGMENT,
		OPEN_INSERT_SEGMENT_POPUP,
		DELETING_SEGMENT,
		OPEN_DELETE_SEGMENT_POPUP,
		HOVERING_SEGMENT,
		HOVERING_SEGMENT_VALUE,
		DRAGGING_SEGMENT_VALUE,
		HOVERING_CONTROL_POINT,
		DRAGGING_CONTROL_POINT,
		HOVERING_TAN_POINT,
		DRAGGING_TAN_POINT,
		NONE
	};

	class SequenceGUIActionData
	{
	public:
		SequenceGUIActionData() {}
		virtual ~SequenceGUIActionData() {}
	};

	enum TanPointTypes
	{
		IN,
		OUT
	};

	class SequenceGUIDragTanPointData : public SequenceGUIActionData
	{
	public:
		SequenceGUIDragTanPointData(std::string trackId_, std::string segmentID_, int controlPointIndex_, TanPointTypes type_)
			: trackID(trackId_), segmentID(segmentID_), controlPointIndex(controlPointIndex_), type(type_) {}

		std::string		trackID;
		std::string		segmentID;
		int				controlPointIndex;
		TanPointTypes	type;
	};

	class SequenceGUIDragControlPointData : public SequenceGUIActionData
	{
	public:
		SequenceGUIDragControlPointData(std::string trackId_, std::string segmentID_, int controlPointIndex_) 
			: trackID(trackId_), segmentID(segmentID_), controlPointIndex(controlPointIndex_){}

		std::string trackID;
		std::string segmentID;
		int			controlPointIndex;
	};

	class SequenceGUIDeleteSegmentData : public SequenceGUIActionData
	{
	public:
		SequenceGUIDeleteSegmentData(std::string trackId_, std::string segmentID_) : trackID(trackId_), segmentID(segmentID_) {}

		std::string trackID;
		std::string segmentID;
	};

	class SequenceGUIInsertSegmentData : public SequenceGUIActionData
	{
	public:
		SequenceGUIInsertSegmentData(std::string id, double t) : trackID(id), time(t) {}

		double time = 0.0;
		std::string trackID;
	};

	class SequenceGUIState
	{
	public:
		SequenceGUIMouseActions currentAction = SequenceGUIMouseActions::NONE;
		std::string currentObjectID = "";
		std::unique_ptr<SequenceGUIActionData> currentActionData;
	};


	/**
	 */
	class SequenceEditorView
	{
	public:
		// constructor
		SequenceEditorView(const Sequence& sequence, SequenceEditorController& controller);
	protected:
		const Sequence& mSequence;
		SequenceEditorController& mController;
	};


	/**
	 */
	class SequenceEditorGUIView : public SequenceEditorView
	{
	public:
		SequenceEditorGUIView(
			const Sequence& sequence,
			SequenceEditorController& controller,
			std::string id);

		void draw();

	private:
		void drawTracks(
			const Sequence &sequence, 
			const float timelineWidth,
			const ImVec2 &mousePos,
			const float stepSize, 
			const ImVec2 &mouseDelta);

		std::string mID;
		SequenceGUIState mState;
		ImVec2 mPreviousMousePos = { 0,0 };
	};


}
