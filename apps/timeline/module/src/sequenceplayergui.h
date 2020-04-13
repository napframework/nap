#pragma once

// internal includes
#include "sequenceplayer.h"

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <rtti/objectptr.h>
#include <imgui/imgui.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequencePlayerGUIView;
	class SequencePlayerView;
	class SequencePlayerGUIActionData;

	/**
	*/
	class NAPAPI SequencePlayerGUI : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual bool init(utility::ErrorState& errorState);

		virtual void onDestroy();

		void draw();
	public:
		ResourcePtr<SequencePlayer> mSequencePlayer = nullptr;
	protected:
		std::unique_ptr<SequencePlayerGUIView> mView = nullptr;
	public:
	};

	namespace SequencePlayerActions
	{
		enum SequencePlayerActions
		{
			// ACTIONS
			HOVERING_PLAYER_TIME,
			DRAGGING_PLAYER_TIME,
			LOAD,
			NONE
		};
	}

	/**
	 *
	 */
	class SequencePlayerGUIState
	{
	public:
		SequencePlayerActions::SequencePlayerActions currentAction = SequencePlayerActions::NONE;
		std::unique_ptr<SequencePlayerGUIActionData> currentActionData = nullptr;
	};

	/**
	 */
	class SequencePlayerGUIView
	{
	public:
		SequencePlayerGUIView(
			SequencePlayer& player,
			std::string id);

		virtual void draw();
	private:
		void drawTracks(
			const SequencePlayer& sequencePlayer,
			const Sequence &sequence);

		template<typename T>
		void drawSegmentContent(
			const SequenceTrack &track,
			const SequenceTrackSegment &segment,
			const ImVec2& trackTopLeft,
			float previousSegmentX,
			float segmentWidth,
			float segmentX,
			ImDrawList* drawList,
			bool drawStartValue);

		template<typename T>
		void drawCurves(
			const SequenceTrack& track,
			const SequenceTrackSegment& segment,
			const ImVec2 &trackTopLeft,
			const float previousSegmentX,
			const float segmentWidth,
			const float segmentX,
			ImDrawList* drawList);

		void drawPlayerController(SequencePlayer& playerm);

		void drawTimelinePlayerPosition(const Sequence& sequence, SequencePlayer& player);

		void handleLoadPopup();
	protected:
		// ImGUI tools
		bool Combo(const char* label, int* currIndex, std::vector<std::string>& values);

		bool ListBox(const char* label, int* currIndex, std::vector<std::string>& values);

		std::string formatTimeString(double time);
	protected:

		std::unordered_map<std::string, std::vector<ImVec2>> mCurveCache;

		SequencePlayer& mPlayer;
	protected:
		SequencePlayerGUIState mPlayerAction;

		std::string mID;
		ImVec2 mPreviousMousePos;

		bool mIsWindowFocused = false;
		ImVec2 mMouseDelta;
		ImVec2 mMousePos;
		ImVec2 mWindowPos;
		ImVec2 mTimelineControllerPos;
		float mTimelineWidth;
		float mStepSize;
		float mTrackHeight;
		float mInspectorWidth;
		ImVec2 mPrevWindowPos;
		ImVec2 mPrevScroll;
		float mVerticalResolution = 100.0f;
		float mHorizontalResolution = 100.0f;
	};

	class SequencePlayerGUIActionData
	{
	public:
		SequencePlayerGUIActionData() {}
		virtual ~SequencePlayerGUIActionData() {}
	};

	class SequencePlayerGUILoadShowData : public SequencePlayerGUIActionData
	{
	public:
		SequencePlayerGUILoadShowData() {}

		int selectedShow = 0;
		std::string errorString;
	};

	class SequenceGUIPlayerDragPlayerData : public SequencePlayerGUIActionData
	{
	public:
		SequenceGUIPlayerDragPlayerData(
			bool playerWasPlaying_,
			bool playerWasPaused_)
			: playerWasPlaying(playerWasPlaying_),
			playerWasPaused(playerWasPaused_) {}

		bool playerWasPlaying;
		bool playerWasPaused;
	};
}
