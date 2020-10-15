/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "cvframe.h"

// External includes
#include <nap/event.h>
#include <memory>

namespace nap
{
	/**
	 * Single video frame event. 
	 * This event occurs when a nap::CVCaptureDevice captures a set of new frames from the adapters registered with it.
	 * This event can therefore contain multiple CV::Frame objects where every frame is captured at exactly 
	 * the same point in time by a nap::CVAdapter. Use the '[]' operator to access the content of a frame. 
	 */
	class NAPAPI CVFrameEvent : public Event
	{
		RTTI_ENABLE(Event)
	public:
		// Default Constructor
		CVFrameEvent() = default;

		// Default copy constructor
		CVFrameEvent(const CVFrameEvent& other);

		// Default copy assignment operator
		CVFrameEvent& operator=(const CVFrameEvent& other);

		/**
		 * Constructor that takes multiple frames as an input argument.
		 * @param frames frames associated with this event.
		 */
		CVFrameEvent(const std::vector<CVFrame>& frames);

		/**
		 * Constructor that takes multiple frames as an input argument.
		 * @param frames frames associated with this event.
		 */
		CVFrameEvent(const std::vector<CVFrame>&& frames);

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
		 * Performs a deep copy of the frames in this event.
		 * The default copy operation does not copy the actual content of the frames, only increases the ref count.
		 * The content of the given event is cleared.
		 * @param outEvent the event to copy the data of this frame to.
		 */
		void copyTo(CVFrameEvent& outEvent) const;

		/**
		 * Returns a clone of the frames in this event.
		 * The default copy operation does not copy the actual content of the frames, only increases the ref count.
		 * @return a clone of this event
		 */
		CVFrameEvent clone() const;

		/**
		 * @return number of frames associated with this event
		 */
		int getCount() const										{ return static_cast<int>(mFrames.size()); }

		/**
		 * Request a change in frame capacity.
		 * @param count number of frames
		 */
		void reserve(int count)										{ mFrames.reserve(count); }

		/**
		 * Pop the last frame from the stack, the frame is destroyed
		 */
		void popFrame()												{ mFrames.pop_back(); }

		/**
		 * @return the last frame
		 */
		CVFrame& lastFrame()										{ return mFrames.back(); }

		/**
		 * @return if the event contains no frame data
		 */
		bool empty() const											{ return mFrames.empty(); }

		/**
		 * @return all the frames associated with this event.
		 */
		const std::vector<CVFrame>& getFrames() const				{ return mFrames; }

		/**
		 * @return the frame at the given index
		 */
		const CVFrame& getFrame(int index) const;

		/**
		 * Finds the frame for the given adapter.
		 * @return the frame associated with the given adapter, nullptr if not found.
		 */
		const CVFrame* findFrame(const CVAdapter& adapter) const;

		/**
		 *	Clears all frames
		 */
		void clear()												{ mFrames.clear(); }

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