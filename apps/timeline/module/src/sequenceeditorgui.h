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
		NONE
	};

	class SequenceGUIStateActionData
	{
	public:
		SequenceGUIStateActionData() {}
		virtual ~SequenceGUIStateActionData() {}
	};

	class SequenceGUIInsertSequenceData : public SequenceGUIStateActionData
	{
	public:
		SequenceGUIInsertSequenceData(std::string id, double t) : trackID(id), time(t) {}

		double time = 0.0;
		std::string trackID;
	};

	class SequenceGUIState
	{
	public:
		SequenceGUIMouseActions currentAction = SequenceGUIMouseActions::NONE;
		std::string currentObjectID = "";
		std::unique_ptr<SequenceGUIStateActionData> currentActionData;
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
		std::string mID;
		SequenceGUIState mState;
		ImVec2 mPreviousMousePos = { 0,0 };
	};


}
