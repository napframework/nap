#pragma once

// Local Includes
#include "glyph.h"

// External Includes
#include <nap/resource.h>
#include <rtti/factory.h>
#include <utility>
#include <utility/errorstate.h>
#include <rect.h>

// Forward Declares
namespace nap
{
	class FontService;
	class FontInstance;
	class GlyphCache;
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
	 * Internally the font caches all the requested characters and visualization modes.
	 * A character is called a Glyph and a character visualization mode is a GlyphRepresentation.
	 * The instance is created by a Font on initialization.
	 * Use the getGlyphIndex() and getOrCreateGlyph() functions to fetch characters.
	 * All the characters (glyphs) are managed and therefore owned by this font.
	 * It is possible to create a font instance dynamically at run-time.
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
		 * @return up to date properties associated with this font
		 */
		const FontProperties& getProperties() const;

		/**
		 * @return if this instance hosts a valid type-face
		 */
		bool isValid() const;

		/**
		 * Get the index of a Glyph inside this font based on a character code
		 * Use this index to get a handle to a Glyph using getOrCreateGlyph()
		 * @param character the character code, 0 when not identified
		 */
		 nap::uint getGlyphIndex(nap::uint character) const;

		/**
		 * Use this to acquire a handle to a glyph representation, associated with a character at that index.
		 * The object can't be copied and is owned by this font. 
		 * When the Glyph isn't present it is created and initialized afterwards.
		 * Use getGlyphIndex() to find the index of a specific character.
		 * T must be of type IGlyphRepresentation, multiple representations of every character are allowed.
		 * This means that you can associate multiple Glyph representations with the same character index.
		 * @param index the index of the glyph inside the font.
		 * @param errorCode contains the error if the glyph could not be created or fetched
		 * @return a glyph associated with a specific character, nullptr if retrieval fails.
		 */
		template<typename T>
		T* getOrCreateGlyphRepresentation(nap::uint index, utility::ErrorState& errorCode);

		/**
		 * Returns a native Glyph object that can be represented using a Glyph Representation object.
		 * The Glyph is cached internally, to speed up future requests.
		 * @param index the index of the glyph inside the font.
		 * @param errorCode contains the error if the glyph could not be created or fetched
		 * @return a Glyph Cache associated with a specific index of the font.
		 */
		const Glyph* getOrCreateGlyph(nap::uint index, utility::ErrorState& errorCode);

		/**
		 * Returns the bounding box in pixels associated with a string of text.
		 * This bounding box tightly wraps the text and excludes initial horizontal bearing
		 * All requested glyphs are cached internally.
		 * @param text the string of characters to compute the bounding box for
		 * @param outRect contains the text bounds
		 */
		void getBoundingBox(const std::string& text, math::Rect& outRect);

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
		void* getGlyphHandle();

		/**
		 * Resets the Glyph cache.
		 * Occurs when a new face is initialized or the size changes
		 */
		void resetCache();

		/**
		 * Gets or creates a new Glyph representation and adds it to the individual glyph cache
		 * @param cache the cache that manages all the glyph representations
		 */
		template<typename T>
		T* getOrCreateRepresentation(GlyphCache& cache, utility::ErrorState& errorCode);

		/**
		* Returns a native Glyph cache object that holds a a glyph and the available glyph presentation modes
		* The Glyph is cached internally, to speed up future requests.
		* @param index the index of the glyph inside the font.
		* @param errorCode contains the error if the glyph could not be created or fetched
		* @return a Glyph Cache associated with a specific index of the font.
		*/
		GlyphCache* getOrCreateGlyphCache(nap::uint index, utility::ErrorState& errorCode);


		void* mFace = nullptr;											///< Handle to the free-type face object
		void* mFreetypeLib = nullptr;									///< Handle to the free-type library
		FontProperties mProperties = { -1, -1, "" };					///< Describes current font properties
		mutable std::vector<std::unique_ptr<GlyphCache>> mGlyphs;		///< All cached glyphs
	};


	//////////////////////////////////////////////////////////////////////////
	// Glyph Cache
	//////////////////////////////////////////////////////////////////////////

	/**
	* Maps a single glyph to multiple glyph representations
	* This object is created and managed by a FontInstance
	*/
	class NAPAPI GlyphCache final
	{
		friend FontInstance;
	public:
		~GlyphCache();

		// Copy is not allowed
		GlyphCache(const GlyphCache& other) = delete;
		GlyphCache& operator=(const GlyphCache&) = delete;

		// Move is not allowed
		GlyphCache(GlyphCache&& other) = delete;
		GlyphCache& operator=(GlyphCache&& other) = delete;

		/**
		* @return a Glyph representation of a specific type, nullptr if not found
		*/
		IGlyphRepresentation* findRepresentation(const rtti::TypeInfo& type);

		/**
		* @return a Glyph representation of type T, nullptr if not found
		*/
		template<typename T>
		T* findRepresentation();

		/**
		* @return parent glyph of this cache
		*/
		const Glyph& getGlyph() const						{ return *mGlyph; }

	protected:
		/**
		* Constructor, only callable by FontInstance
		* @param parent parent Glyph
		*/
		GlyphCache(std::unique_ptr<Glyph> parent) : 
			mGlyph(std::move(parent))						{ }

		/**
		* Adds a new glyph representation to the cache.
		* This cache owns the new representation.
		*/
		void addRepresentation(std::unique_ptr<IGlyphRepresentation> representation);

	private:
		std::unique_ptr<Glyph> mGlyph = nullptr;			///< Pointer to parent glyph object
		using GlyphRepresentationMap = std::unordered_map<rtti::TypeInfo, std::unique_ptr<IGlyphRepresentation>>;
		GlyphRepresentationMap mRepresentations;			///< All the possible ways the glyph can be represented
	};


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	T* nap::FontInstance::getOrCreateGlyphRepresentation(nap::uint index, utility::ErrorState& errorCode)
	{
		assert(isValid());
		assert(RTTI_OF(T).is_derived_from(RTTI_OF(IGlyphRepresentation)));

		// Acquire handle to glyph cache
		GlyphCache* cache = getOrCreateGlyphCache(index, errorCode);
		if (cache == nullptr)
			return nullptr;

		// Add a new representation
		return getOrCreateRepresentation<T>(*cache, errorCode);
	}


	template<typename T>
	T* nap::FontInstance::getOrCreateRepresentation(GlyphCache& cache, utility::ErrorState& errorCode)
	{
		// Find requested representation of this glyph
		T* representation = cache.findRepresentation<T>();
		if (representation != nullptr)
			return representation;

		// Add new representation and move to unique ptr
		T* new_rep = new T();
		std::unique_ptr<IGlyphRepresentation> urep(new_rep);

		// Initialize it
		if (!urep->init(cache.getGlyph(), errorCode))
			return nullptr;

		// Add to cache
		cache.addRepresentation(std::move(urep));
		return new_rep;
	}


	template<typename T>
	T* nap::GlyphCache::findRepresentation()
	{
		IGlyphRepresentation* presentation = findRepresentation(RTTI_OF(T));
		return rtti_cast<T>(presentation);
	}
}
