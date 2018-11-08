#pragma once

#include <nap/numeric.h>
#include <rtti/typeinfo.h>
#include <utility/errorstate.h>
#include <glm/glm.hpp>

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
		uint getIndex() const									{ return mIndex; }

		/**
		 * @return if the Glyph represents a valid symbol / character
		 */
		bool isValid();

		/**
		 * @return horizontal glyph advance value in pixels
		 */
		int getHorizontalAdvance() const						{ return mAdvance.x; }

		/**
		 * @return vertical glyph advance value in pixels
		 */
		int getVerticalAdvance() const							{ return mAdvance.y; }

	protected:
		/**
		 * Only a font instance can create a glyph
		 * @param handle to the glyph in memory, should be of type FT_Glyph
		 * @param index the index of the glyph in the font
		 */
		Glyph(void* handle, uint index);

		/**
		 * Offers additional initialization options, implement in derived classes.
		 * Only a font can initialize a glyph, init() is called directly after construction,
		 * If initialization fails the Glyph is not added to the cache!
		 * @param errorCode contains the error if initialization fails
		 * @return if initialization succeeds
		 */
		virtual bool init(utility::ErrorState& errorCode)	{ return true; }

		/**
		 * @return handle to the free-type glyph object. Always of type: FT_Glyph!
		 */
		inline void* getHandle() const						{ return mHandle; }

	private:
		void*		mHandle = nullptr;			///< Handle to the Glyph in memory
		uint		mIndex = 0;					///< Index of the Glyph inside the font
		glm::ivec2	mAdvance = { -1, -1 };		///< Offset in pixels to advance to next glyph
	};
}

