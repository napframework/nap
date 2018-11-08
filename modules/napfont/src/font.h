#pragma once

// Local Includes
#include "glyph.h"

// External Includes
#include <nap/resource.h>
#include <rtti/factory.h>
#include <unordered_map>
#include <utility>
#include <utility/errorstate.h>

// Forward Declares
namespace nap
{
	class FontService;
	class FontInstance;
}


namespace nap
{
	/**
	 * Simple struct that describes common properties of a font
	 * This struct can be copied easily and is used by all font related classes
	 */
	struct NAPAPI FontProperties
	{
		/**
		 * Default constructor
		 */
		FontProperties() = default;

		/**
		 * Value constructor
		 */
		FontProperties(int size, int dpi, const std::string& font) :
			mSize(size),
			mDPI(dpi),
			mFont(font)			{ }

		int mSize	= 12;			///< Property: 'Size' size of the font in em (points)
		int mDPI	= 96;			///< Property: "DPI' dots per inch of the monitor, typically 96 or 72
		std::string	mFont;			///< Property: 'Font' path to the font on disk
	};


	//////////////////////////////////////////////////////////////////////////
	// Font Resource
	//////////////////////////////////////////////////////////////////////////


	/**
	 * Font Resource
	 */
	class NAPAPI Font : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * Constructor used by factory registered in the font service
		 */
		Font(FontService& service);

		// Destructor
		virtual ~Font();

		/**
		 * Initialize this object after de-serialization, creates a new font instance.
		 * Use the font instance to access all font related information
		 * @param errorState contains the error message when initialization fails
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Returns the font created on initialization, use this object to access all font related functionality
		 * @return the font created on initialization
		 */
		FontInstance& getFontInstance();

		/**
		* Returns the font created on initialization, use this object to access all font related functionality
		* @return the font created on initialization
		*/
		const FontInstance& getFontInstance() const;

		FontProperties mProperties;								///< Property: 'Properties' the properties (size, dpi, path) that describe the font

	private:
		FontService* mService = nullptr;						///< Handle to the service that manages the font library
		std::unique_ptr<FontInstance> mInstance = nullptr;		///< Font instance created by this resource
	};

	// Object creator used for constructing the the OSC receiver
	using FontObjectCreator = rtti::ObjectCreator<Font, FontService>;


	//////////////////////////////////////////////////////////////////////////
	// Font Instance
	//////////////////////////////////////////////////////////////////////////


	/**
	 * Runtime version of a font that serves characters.
	 * Internally the font caches all the requested characters.
	 * The instance is created by a Font on initialization.
	 * Use the getGlyphIndex() and getOrCreateGlyph() functions to fetch characters.
	 * All the characters (glyphs) are managed and therefore owned by this font.
	 * It is possible to create a font instance dynamically at run-time.
	 * When you change font properties the cache is automatically cleared
	 */
	class NAPAPI FontInstance final
	{
		RTTI_ENABLE()
	public:
		/**
		 * Create the instance based on font properties and a service
		 */
		FontInstance(const FontService& service);

		/**
		 *	Destructor
		 */
		~FontInstance();

		/**
		 * Font can't be copied!
		 */
		FontInstance(const FontInstance& rhs) = delete;
		FontInstance& operator=(const FontInstance& rhs) = delete;

		/**
		 * Creates the actual type-face associated with the font based on the incoming set of properties.
		 * Call this after construction or when you want to change the font at run-time.
		 * If for some reason construction fails the errorState holds the error code.
		 * The previous type-face, if it exists, is destroyed when a new one is created successfully.
		 * When creation fails the current type-face remains active.
		 * @param error contains the error message if construction of the typeface fails
		 * @return if the typeface could be constructed
		 */
		bool create(const FontProperties& properties, utility::ErrorState& error);

		/**
		 * Allows for changing the size of the font at run-time
		 * Updates the size and dpi variables and pushes the size changes to the managed typeface
		 * Call create() before calling this function!
		 * @param size the new size of the font in em (points)
		 * @param dpi the dots per inch (resolution) of the monitor
		 * @return if the new size could be set, ie: is supported by the type face
		 */
		bool changeSize(int size, int dpi);

		/**
		 * @return up to date properties associated with this font
		 */
		const FontProperties& getProperties() const;

		/**
		 * @return if this instance hosts a valid type-face
		 */
		bool isValid() const;

		/**
		 *	Get the index of a Glyph inside this font based on a character code
		 * You can use this index in conjunction with getGlyph()
		 * @param character the character code, 0 when not identified
		 */
		 nap::uint getGlyphIndex(nap::uint character) const;

		/**
		 * Use this to acquire a handle to a glyph of type T that is associated with a specific index.
		 * The Glyph can't be copied and is owned by this font. 
		 * It's better not to store the handle but ask for one when you need it!
		 * When the Glyph isn't present it is created and initialized afterwards.
		 * Use getGlyphIndex() to find the index of a specific character.
		 * T must be of type Glyph. The font does not handle interleaved Glyph types!
		 * This means that you cannot associated 2 different types of Glyphs with the same index.
		 * @param index the index of the glyph inside the font.
		 * @param errorCode contains the error if the glyph could not be created or fetched
		 * @return a glyph associated with a specific character, nullptr if retrieval fails.
		 */
		template<typename T>
		T* getOrCreateGlyph(nap::uint index, utility::ErrorState& errorCode);

		/**
		 * Returns the number of glyphs in this font, -1 when the font hasn't been created yet
		 * @return the number of glyphs associated with this font
		 */
		int getCount();

	protected:
		/**
		 * Returns the handle to the free-type face managed by this resource
		 * Represents a FT_Face object
		 * @return handle to the type face managed by this font
		 */
		void* getFace() const;

	private:
		/**
		 * Load a glyph into the glyph slot of this font.
		 * @param the index of the glyph inside the font
		 * @return if a new glyph is loaded correctly
		 */
		bool loadGlyph(uint index);

		/**
		 * Get a copy of the currently loaded glyph, ready for caching
		 * @return handle to the copied glyph in memory, nullptr on error
		 */
		void* getGlyph();

		/**
		 * Resets the Glyph cache.
		 * Occurs when a new face is initialized or the size changes
		 */
		void resetCache();
		
		void* mFace = nullptr;											///< Handle to the free-type face object
		void* mFreetypeLib = nullptr;									///< Handle to the free-type library
		FontProperties mProperties = { -1, -1, "" };					///< Describes current font properties

		using GlyphMap = std::unordered_map<uint, std::unique_ptr<Glyph>>;
		GlyphMap mGlyphs;												///< All cached glyphs
	};


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	T* nap::FontInstance::getOrCreateGlyph(nap::uint index, utility::ErrorState& errorCode)
	{
		assert(isValid());
		assert(RTTI_OF(T).is_derived_from(RTTI_OF(Glyph)));

		// Try to find a cached Glyph
		auto it = mGlyphs.find(index);
		if (it != mGlyphs.end())
		{
			T* c_glyph = rtti_cast<T>(it->second.get());
			assert(c_glyph != nullptr);
			return c_glyph;
		}

		// Load a new glyph
		if (!errorCode.check(loadGlyph(index), "unable to load glyph: %d into memory", index))
			return nullptr;

		// Copy handle
		void* glyph_handle = getGlyph();
		if (!errorCode.check(glyph_handle != nullptr, "unable to get handle to glyph: %d", index))
			return nullptr;

		// Create new glyph
		T* ptr = new T(glyph_handle, index);
		std::unique_ptr<T> uptr(ptr);

		// Initialize, when initialization fails the uptr is destroyed automatically
		if (!uptr->init(errorCode))
			return nullptr;

		// Store
		mGlyphs.emplace(std::make_pair(index, std::move(uptr)));
		return ptr;
	}
}
