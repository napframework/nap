#pragma once

// External Includes
#include <nap/resource.h>
#include <rtti/factory.h>

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
	struct FontProperties
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
		* Initialize this object after de-serialization
		* This creates the type face object
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

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
	 * Runtime version of a font
	 * Allows for dynamically changing font size and font type
	 * The instance is created by a Font on initialization
	 * You can also create a font instance dynamically at run-time
	 */
	class FontInstance final
	{
		RTTI_ENABLE()
	public:
		/**
		 * Create the instance based on font properties and a service
		 */
		FontInstance(const FontProperties& properties, const FontService& service);

		/**
		 *	Destructor
		 */
		~FontInstance();

		/**
		 * Creates the actual type-face associated with the font
		 * Call this after construction of this object or after updating the font properties
		 * If for some reason construction fails the errorState holds the error code
		 * @param error contains the error message if construction of the typeface fails
		 * @return if the typeface could be constructed
		 */
		bool create(utility::ErrorState& error);

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
		 * @return properties associated with this font
		 */
		const FontProperties& getProperties() const;

		/**
		 * @return if this instance hosts a valid type-face
		 */
		bool isValid() const;

	protected:
		/**
		* Returns the handle to the free-type face managed by this resource
		* Represents a FT_Face object
		* @return handle to the type face managed by this font
		*/
		void* getFace() const;

	private:

		void* mFace = nullptr;							///< Handle to the free-type face object
		void* mFreetypeLib = nullptr;					///< Handle to the free-type library
		FontProperties mProperties = { -1, -1, "" };	///< Describes current font properties
	};
}
