#pragma once

#include <nap/numeric.h>
#include <rtti/typeinfo.h>
#include <utility/errorstate.h>
#include <glm/glm.hpp>

namespace nap
{
	class FontInstance;
	class Core;

	/**
	 * Represents a symbol (character) in a font.
	 * This class wraps and manages 1 free-type Glyph character and is created by a FontInstance.
	 * The Glyph is destroyed when this object is destructed.
	 * Every Glyph can be represented using an IGLyphRepresentation.
	 * This allows for multiple interpretations of the same Glyph, for example a 2DTexture or 3Dmesh.
	 */
	class NAPAPI Glyph final
	{
		friend FontInstance;
		RTTI_ENABLE()
	public:
		
		// Default construction is not allowed
		Glyph() = delete;

		/**
		 * Destructor, unloads and destroys the glyph if present
		 */
		~Glyph();

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

		/**
		 * @return handle to the free-type glyph object. Always of type: FT_Glyph!
		 */
		inline void* getHandle() const							{ return mHandle; }

	protected:
		/**
		 * Only a font instance can create a glyph
		 * @param handle to the glyph in memory, should be of type FT_Glyph
		 * @param index the index of the glyph in the font
		 */
		Glyph(void* handle, uint index);

	private:
		void*		mHandle = nullptr;			///< Handle to the Glyph in memory
		uint		mIndex = 0;					///< Index of the Glyph inside the font
		glm::ivec2	mAdvance = { -1, -1 };		///< Offset in pixels to advance to next glyph
	};


	//////////////////////////////////////////////////////////////////////////
	// IGlyphRepresentation
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Interface of a specific type of Glyph representation.
	 * This could be an image or 3D mesh constructed from a parent Glyph.
	 * Override the onInit method to create your own Glyph representation.
	 * This class does not own or manage a Glyph, it only allows you to build a representation.
	 * Glyph representations can only be created by a FontInstance.
	 */
	class NAPAPI IGlyphRepresentation
	{
		friend FontInstance;
		RTTI_ENABLE()
	public:
		/**
		 * Destructor, unloads and destroys the glyph if present
		 */
		virtual ~IGlyphRepresentation()							{ }

		/**
		 * Constructor
		 */
		IGlyphRepresentation(nap::Core& core);

		// Copy is not allowed
		IGlyphRepresentation(const IGlyphRepresentation& other) = delete;
		IGlyphRepresentation& operator=(const IGlyphRepresentation&) = delete;

		// Move is not allowed
		IGlyphRepresentation(IGlyphRepresentation&& other) = delete;
		IGlyphRepresentation& operator=(IGlyphRepresentation&& other) = delete;

	protected:
		/**
		 * Called by the font when a specific Glyph representation is requested.
		 * Simply forwards the call to the derived onInit implementation.
		 * @param glyph the parent glyph, associated with this glyph representation.
		 * @param error contains the error if initialization fails.
		 * @return if initialization succeeded or failed.
		 */
		bool init(const Glyph& glyph, utility::ErrorState& error)						{ return onInit(glyph, error); }

		/**
		 * Override this method to implement a specific Glyph representation.
		 * The handle to the parent Glyph can't be copied and is owned by the Font.
		 * @param glyph the parent glyph, associated with this glyph representation.
		 * @param error contains the error if initialization fails.
		 * @return if initialization succeeded or failed.
		 */
		virtual bool onInit(const Glyph& glyph, utility::ErrorState& error) = 0;

		nap::Core* mCore;	///< Handle to core instance
	};
}

