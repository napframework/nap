#include "timelinestate.h"
#include <ctime>

namespace nap
{
	namespace emography
	{
		TimelineState::TimelineState()
		{
			// Get current date (without time)
			std::time_t curtime;
			std::time(&curtime);

			std::tm dayStart = *(std::localtime(&curtime));
			dayStart.tm_hour = 0;
			dayStart.tm_min = 0;
			dayStart.tm_sec = 0;

			std::tm dayEnd = *(std::localtime(&curtime));
			dayEnd.tm_hour = 23;
			dayEnd.tm_min = 59;
			dayEnd.tm_sec = 59;

			uint64_t dayStartTime = std::mktime(&dayStart);
			uint64_t dayEndTime = std::mktime(&dayEnd);

			mTimeLeft = dayStartTime;
			mTimeRight = dayEndTime;
		}

		bool TimelineState::OnMouseDown(const glm::vec2& inPosition, bool inCtrlDown, bool inAltDown)
		{
			//  In case an active mode was set, it means it is a mode set by the UI buttons. We store it here
			// so that we can test later if the mode was overridden by a shortcut key
			mPrevActiveMode = mActiveMode;

			mCursorClickPos = inPosition;

			mDragStartLeftTime = mTimeLeft;
			mDragStartRightTime = mTimeRight;
			mLastCursorTimePos = PixelToAbsTime(inPosition.x);
			mCursorClickTimePos = mLastCursorTimePos;

			// If the active mode is set through the UI buttons, it can be overridden by any of the shortcut keys
			if (inCtrlDown)
				SetActiveMode(EMode::Pan);
			else if (inAltDown)
				SetActiveMode(EMode::Zoom);

			SetModeState(EModeState::Started);

			return true;
		}

		void TimelineState::OnMouseEnter(const glm::vec2& inPosition, bool inCtrlDown, bool inAltDown)
		{

		}

		void TimelineState::OnMouseLeave(const glm::vec2& inPosition, bool inCtrlDown, bool inAltDown)
		{
			mLastCursorTimePos = -1;
			SetModeState(EModeState::Finished);
		}

		void TimelineState::OnMouseMove(const glm::vec2& inPosition, bool inCtrlDown, bool inAltDown)
		{
			if (IsModeStarted())
			{
				glm::vec2 drag_delta = mCursorClickPos - inPosition;

				// If a button is released, we should cancel and revert back to selection mode. If mPrevActiveMode is set, 
				// it means we entered a mode through the UI buttons. In that case, we should not revert the mode if a 
				// button is released.
				const bool modeActivatedThroughUI = mPrevActiveMode != EMode::Select;

				if (mActiveMode == EMode::Pan)
				{
					if (inCtrlDown || modeActivatedThroughUI)
					{
						double total_time = mDragStartRightTime - mDragStartLeftTime;
						float time_moved = (float)((drag_delta.x / mWidthInPixels) * total_time);

						MoveTo(mDragStartLeftTime + (uint64_t)time_moved);
					}
					else
					{
						SetModeState(EModeState::Finished);
						SetActiveMode(EMode::Select);
					}
				}
				else if (mActiveMode == EMode::Zoom)
				{
					if (inAltDown || modeActivatedThroughUI)
					{
						const float mouseMoveMultiplier = 0.001f + powf(0.5f, 3.0f) * 0.05f;
						ZoomAroundTime(mCursorClickTimePos, mDragStartLeftTime, mDragStartRightTime, drag_delta.x * mouseMoveMultiplier);
					}
					else
					{
						SetModeState(EModeState::Finished);
						SetActiveMode(EMode::Select);
					}
				}
			}

			mLastCursorTimePos = PixelToAbsTime(inPosition.x);
		}

		void TimelineState::OnMouseUp(const glm::vec2& inPosition, bool inCtrlDown, bool inAltDown)
		{
			if (IsModeStarted())
			{
				// If the active mode is overridden by a shortcut, we always go back to select mode.
				if (mActiveMode != mPrevActiveMode)
					SetActiveMode(EMode::Select);

				SetModeState(EModeState::Finished);
			}

			mLastCursorTimePos = PixelToAbsTime(inPosition.x);
		}

		double TimelineState::GetClampedRange(double inTimeRangeMs)
		{
			return glm::clamp<double>(mMinTimeRangeMs, mMaxTimeRangeMs, inTimeRangeMs);
		}

		void TimelineState::ZoomAroundTime(uint64_t inClickTimePos, uint64_t inTimeLeft, uint64_t inTimeRight, float inDelta)
		{
			double exp = std::log10(inTimeRight - inTimeLeft);
			exp += inDelta;
			double range = std::pow(10, exp);
			range = GetClampedRange(range);

			double scale = ((double)inClickTimePos - (double)inTimeLeft) / ((double)inTimeRight - (double)inTimeLeft);

			double newTimeLeft = inClickTimePos - scale * range;
			double newTimeRight = newTimeLeft + range;

			mTimeLeft = newTimeLeft;
			mTimeRight = newTimeRight;
		}

		void TimelineState::MoveTo(uint64_t inStartPosition)
		{
			uint64_t curRange = mDragStartRightTime - mDragStartLeftTime;
			uint64_t endPosition = inStartPosition + curRange;
			uint64_t newStartPosition = inStartPosition;
			uint64_t newEndPosition = endPosition;

			mTimeLeft = newStartPosition;
			mTimeRight = newEndPosition;
		}
	}
}