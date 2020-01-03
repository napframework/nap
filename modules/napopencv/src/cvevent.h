#pragma once

// Local includes
#include "cvframe.h"

// External includes
#include <nap/event.h>
#include <memory>

namespace nap
{
	/**
	 * Single video frame event. This event occurs when one or more OpenCV video devices capture a new frame.
	 * This event can contain multiple frames, where every frame is captured at the same point in time.
	 */
	class NAPAPI CVFrameEvent : public Event
	{
		RTTI_ENABLE(Event)
	public:
		/**
		 * Constructs a frame event with the given number of frames.
		 * @param count number of frames.
		 */
		CVFrameEvent(int count);

		// Default Constructor
		CVFrameEvent() = default;

		/**
		 * Constructor that takes a frame as input argument.
		 * @param frame frame associated with this event.
		 */
		CVFrameEvent(const CVFrame& frame);
		
		/**
		 * Constructor that takes a frame as input argument, the input is moved.
		 * @param frame frame associated with this event.
		 */
		CVFrameEvent(CVFrame&& frame);

		/**
		 * Adds a new frame to this event.
		 * @param frame the frame to add to this event
		 */
		void addFrame(const CVFrame& frame);

		/**
		 * Adds a new frame to this event, the input is moved.
		 * @param frame the frame to add to this event
		 */
		void addFrame(CVFrame&& frame);

		/**
		 * @return number of frames associated with this event
		 */
		int getCount() const										{ return static_cast<int>(mFrames.size()); }

		/**
		 * @return all the frames associated with this event.
		 */
		const std::vector<CVFrame>& getFrames() const				{ return mFrames; }

		/**
		 * @return the frame at the given index
		 */
		const CVFrame& getFrame(int index);

		/**
		 * Array subscript overload. Does not perform a bounds check!
		 * @return the frame at the given index
		 */
		CVFrame& operator[](std::size_t index)						{ return mFrames[index]; }

		/**
		 * Array subscript overload. Does not perform a bounds check!
		 * @return the frame at the given index
		 */
		const CVFrame& operator[](std::size_t index) const			{ return mFrames[index]; }

	private:
		std::vector<CVFrame> mFrames;				///< All the frames associated with this event
	};

	using CVFrameEventPtr = std::unique_ptr<CVFrameEvent>;
}