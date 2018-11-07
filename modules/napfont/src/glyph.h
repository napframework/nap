#pragma once

#include <nap/numeric.h>
#include <rtti/typeinfo.h>

namespace nap
{
	class FontInstance;

	/**
	 * Represents a symbol (character) in a font
	 * This class wraps and manages 1 free-type Glyph character
	 * The Glyph is destroyed when this object is destructed
	 */
	class NAPAPI Glyph
	{
		friend FontInstance;
		RTTI_ENABLE()
	public:
		
		// Default construction is not allowed
		Glyph() = delete;

		/**
		* Destructor, unloads and destroys the glyph if present
		*/
		virtual ~Glyph();

		// Copy is not allowed
		Glyph(const Glyph& other) = delete;
		Glyph& operator=(const Glyph&) = delete;

		// Move is not allowed
		Glyph(Glyph&& other) = delete;
		Glyph& operator=(Glyph&& other) = delete;

		/**
		 * @return the index of the Glyph inside the font
		 */
		uint getIndex() const			{ return mIndex; }

		/**
		 * @return if the Glyph represents a valid symbol / character
		 */
		bool isValid();

	protected:
		/**
		 * Only a font instance can create a glyph
		 * @param handle to the glyph in memory, should be of type FT_Glyph
		 * @param index the index of the glyph in the type face
		 */
		Glyph(void* slot, uint index);

	private:
		void*	mSlot = nullptr;		///< Handle to the Glyph in memory
		uint	mIndex = 0;				///< Index of the Glyph inside the font
	};
}

