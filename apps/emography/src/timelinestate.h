#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace nap
{
	namespace emography
	{
		struct TimelineState
		{
			enum class EMode : uint8_t
			{
				Select,
				Zoom,
				Pan
			};

			enum class EModeState : uint8_t
			{
				Started,
				Finished
			};

			TimelineState();

			inline float AbsTimeToPixel(uint64_t inTime) const
			{
				double total_time = mTimeRight - mTimeLeft;
				double pixels_per_second = mWidthInPixels / total_time;

				double scroll_offset = mTimeLeft * pixels_per_second;

				return (float)(inTime * pixels_per_second - scroll_offset);
			}

			inline uint64_t PixelToTime(float inXPos) const
			{
				double total_time = mTimeRight - mTimeLeft;
				return (inXPos / mWidthInPixels) * total_time;
			}

			inline uint64_t PixelToAbsTime(float inXPos) const
			{
				uint64_t result = mTimeLeft + PixelToTime(inXPos);
				return glm::clamp(mTimeLeft, mTimeRight, result);
			}

			bool OnMouseDown(const glm::vec2& inPosition, bool inCtrlDown, bool inAltDown);

			void OnMouseEnter(const glm::vec2& inPosition, bool inCtrlDown, bool inAltDown);

			void OnMouseLeave(const glm::vec2& inPosition, bool inCtrlDown, bool inAltDown);

			void OnMouseMove(const glm::vec2& inPosition, bool inCtrlDown, bool inAltDown);
			
			void OnMouseUp(const glm::vec2& inPosition, bool inCtrlDown, bool inAltDown);
			
			double GetClampedRange(double inTimeRangeMs);

			void ZoomAroundTime(uint64_t inClickTimePos, uint64_t inTimeLeft, uint64_t inTimeRight, float inDelta);

			void MoveTo(uint64_t inStartPosition);

			void SetActiveMode(EMode inMode)
			{
				mActiveMode = inMode;
			}

			void SetModeState(EModeState inModeState)
			{
				mModeState = inModeState;
			}

			bool IsModeStarted() const { return mModeState == EModeState::Started; }

			uint64_t		mTimeLeft = 0;
			uint64_t		mTimeRight = 1000;
			uint64_t		mMinTimeRangeMs = 1;				// 1 sec
			uint64_t		mMaxTimeRangeMs = 60 * 60 * 24 * 7;	// 7 days
			int				mWidthInPixels = 0;

		private:
			EMode			mPrevActiveMode = EMode::Select;
			EMode			mActiveMode = EMode::Select;
			EModeState		mModeState = EModeState::Finished;
			uint64_t		mDragStartLeftTime = 0;
			uint64_t		mDragStartRightTime = 0;
			uint64_t		mCursorClickTimePos = 0;
			uint64_t		mLastCursorTimePos = -1;
			glm::vec2		mCursorClickPos;
		};
	}
}
