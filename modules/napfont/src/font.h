#pragma once

// External Includes
#include <nap/resource.h>
#include <rtti/factory.h>

// Forward Declares
namespace nap
{
	class FontService;
}


namespace nap
{
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

		/**
		 * Allows for changing the size of the font at run-time
		 * Updates the size and dpi variables and pushes the size changes to the managed typeface
		 * Call init() before calling this function!
		 * @param size the new size of the font in em (points)
		 * @param dpi the dots per inch (resolution) of the monitor
		 * @return if the new size could be set, ie: is supported by the type face
		 */
		bool setSize(int size, int dpi);

		std::string mFont;					///< Property: 'Font' path to the font on disk
		int	mSize = 12;						///< Property: 'Size' size of the font in em (points)
		int mDPI = 96;						///< Property: "DPI' dots per inch of the monitor, typically 96 or 72

	protected:
		/**
		 * Returns the handle to the free-type face managed by this resource
		 * Represents a FT_Face object
		 * @return handle to the type face managed by this font
		 */
		void* getFace() const;

	private:
		void* mFace = nullptr;				///< Handle to the free-type face object
		void* mFreetypeLib = nullptr;		///< Handle to the free-type library
		FontService* mService = nullptr;	///< Handle to the service that manages the font library
	};

	// Object creator used for constructing the the OSC receiver
	using FontObjectCreator = rtti::ObjectCreator<Font, FontService>;
}
