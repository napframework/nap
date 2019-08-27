#pragma once

#include <sequenceelement.h>
#include <sequence.h>
#include <imgui/imgui.h>
#include <string>
#include <mathutils.h>
#include <fcurve.h>
#include <flexblocksequence.h>

#include "flexblockgui.h"

namespace nap
{
	enum TimeLineActions
	{
		NONE = 0,
		DRAGGING_ELEMENT = 1,
		DRAGGING_CURVEPOINT = 2,
		DRAGGING_TANGENT = 3,
		DRAGGING_MOTORVALUE = 4,
		ADD_CURVEPOINT = 5,
		DELETE_CURVEPOINT = 6,
		DRAGGING_PLAYERPOSITION = 7,
		ELEMENT_ACTION_POPUP = 8,
		ENABLING_HANDLERS = 9,
		SEQUENCE_ACTION_POPUP = 10,
		INSERTION_POPUP = 11,
		SAVE_POPUP = 12,
		LOAD_POPUP = 13,
		EDIT_MOTORVALUE_POPUP = 14,
		DRAGGING_SPECIALVALUE = 15,
		EDIT_SPECIALVALUE_POPUP = 16
	};

	class TimelineGuiProperties
	{
	public:
		TimelineGuiProperties() {}
		~TimelineGuiProperties() {}

		float mLengthPerSecond = 60.0f;
		float mChildWidth = 1000.0f;
		float mChildHeight = 350.0f;
		bool mFollowPlayer = false;
		int mCurveResolution = 75;
		timeline::SequenceElement* mSelectedElement = nullptr;
		int mCurrentSelectedMotor = 0;
		int mCurrentSelectedSpecial = 0;
		int mSelectedShowIndex = 0;
		bool mInPopup = false;
		int mMotorHandlerIndexMask = 0;
		int mSpecialsHandlerIndexMask = 0;
		float mCurrentTimeOfMouseInSequence = 0.0f;
		std::string mErrorString;
		timeline::Sequence* mSelectedSequence = nullptr;
		bool mShowToolTips = true;
		ImVec2 mTopLeftPosition;
		bool mDrawMouseCursorInTimeline = false;
		float mMouseCursorPositionInTimeline = 0.0f;
		bool mDirty = true;
		bool mSpecialsDirty = true;
		std::vector<std::vector<ImVec2>> mCachedMotorsCurve;
		std::vector<std::vector<ImVec2>> mCachedSpecialsCurve;
		float mPrevScrollX = 0.0f;
		float mPrevScrollY = 0.0f;
		TimeLineActions mCurrentAction = TimeLineActions::NONE;
		math::FCurvePoint<float, float> *mCurvePtr = nullptr;
		math::FComplex<float, float> *mTangentPtr = nullptr;
		

		std::map<flexblock::PARAMETER_IDS, std::string> mParameterMap =
		{
			{ flexblock::MOTOR_ONE, "m1" },
			{ flexblock::MOTOR_TWO, "m2" },
			{ flexblock::MOTOR_THREE, "m3" },
			{ flexblock::MOTOR_FOUR, "m4" },
			{ flexblock::MOTOR_FIVE, "m5" },
			{ flexblock::MOTOR_SIX, "m6" },
			{ flexblock::MOTOR_SEVEN, "m7" },
			{ flexblock::MOTOR_EIGHT, "m8" },
			{ flexblock::SLACK, "SL" },
			{ flexblock::MOTOR_OVERRIDE_ONE, "o1" },
			{ flexblock::MOTOR_OVERRIDE_TWO, "o2" },
			{ flexblock::MOTOR_OVERRIDE_THREE, "o3" },
			{ flexblock::MOTOR_OVERRIDE_FOUR, "o4" },
			{ flexblock::MOTOR_OVERRIDE_FIVE, "o5" },
			{ flexblock::MOTOR_OVERRIDE_SIX, "o6" },
			{ flexblock::MOTOR_OVERRIDE_SEVEN, "o7" },
			{ flexblock::MOTOR_OVERRIDE_EIGHT, "o8" },
			{ flexblock::SINUS_AMPLITUDE, "SA" },
			{ flexblock::SINUS_FREQUENCY, "SF" },
		};

		bool mShowMotorControl = false;
		bool mShowInformation = true;
		bool mShowParameters = true;
		bool mShowTimeLine = true;
		bool mShowPlaylist = false;
	};
}
