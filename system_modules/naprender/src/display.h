/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <string>
#include <rect.h>
#include <utility/dllexport.h>
#include <nap/numeric.h>

// Forward declares
struct SDL_Window;

namespace nap
{
	/**
	 * Display information.
	 *
	 * The information provided by this interface isn't required to remain valid.
	 * When a display is re-connected, or even turned on/off, the index is re-assigned and this object becomes invalid.
	 */
	class NAPAPI Display final
	{
	public:
		/**
		 * Display orientation modes
		 */
		enum class EOrientation : int8
		{
			Unknown				= 0,
			Landscape			= 1,
			LandscapeFlipped	= 2,
			Portrait			= 3,
			PortraitFlipped		= 4
		};

		/**
		 * Construct a default, invalid display
		 */
		Display() = default;

		/**
		 * Construct display information, index must be >= 0.
		 * @param index display index
		 */
		Display(int index);

		/**
		 * If information is available.
		 * @return if information is available.
		 */
		bool isValid() const { return mValid; }

		/**
		 * Current display index, as given on construction.
		 * Note that his index becomes invalid when the display is re-connected.
		 * 
		 * @return display index as provided on construction
		 */
		int getIndex() const { return mIndex; }

		/**
		 * Returns display name, empty if not available.
		 * @return display name, empty if not available.
		 */
		const std::string& getName() const { return mName; }

		/**
		 * @return min location of desktop area of this display, with the primary display located at 0,0
		 */
		const glm::ivec2& getMin()	const { return mMin; }

		/**
		 * @return max location of desktop area of this display, with the primary display located at 0,0
		 */
		const glm::ivec2& getMax() const { return mMax; }

		/**
		 * @return desktop area of this display, with the primary display located at 0,0
		 */
		math::Rect getBounds() const { return { glm::vec2(mMin), glm::vec2(mMax) }; }

		/**.
		 * Get the content scale of a display.
		 * 
		 * The content scale is the expected scale for content based on the DPI settings of the display.
		 * For example, a 4K display might have a 2.0 (200%) display scale,
		 * which means that the user expects UI elements to be twice as big on this display, to aid in readability.
		 * @return content scale of the display
		 */
		float getContentScale() const { return mScale; }

		/**
		 * Returns the approximated display DPI: 96 * 'display content scale'
		 * @return Approximated display DPI.
		 */
		float getDPI() const { return mDPI; }

		/**
		 * @return display orientation
		 */
		EOrientation getOrientation() const { return mOrientation; }

		/**
		 * Deprecated: use Display::getDPI() instead.
		 * @return dpi
		 */
		[[deprecated]]
		float getDiagonalDPI() const { return mDPI; }

		/**
		 * Deprecated: use Display::getDPI() instead.
		 * @return dpi
		 */
		[[deprecated]]
		float getHorizontalDPI() const { return mDPI; }

		/**
		 * Deprecated: use Display::getDPI() instead.
		 * @return dpi
		 */
		[[deprecated]]
		float getVerticalDPI() const { return mDPI; }

		/**
		 * @return display information as human readable string
		 */
		std::string toString() const;

		/**
		 * @return if two displays are the same based on hardware index.
		 */
		bool operator== (const Display& rhs) const { return rhs.getIndex() == this->getIndex(); }

		/**
		 * @return if two displays values are not the same based on hardware index
		 */
		bool operator!=(const Display& rhs) const { return !(rhs == *this); }

	private:
		std::string mName;						///< Display name
		int mIndex = -1;						///< Display index
		float mDPI = 96.0f;						///< Display DPI
		float mScale = 1.0f;					///< Display content scale
		glm::ivec2 mMin = { 0, 0 };				///< Min display bound position
		glm::ivec2 mMax = { 0, 0 };				///< Max display bound position
		EOrientation mOrientation;				///< Display orientation
		bool mValid = false;					///< If valid after construction
	};
}


