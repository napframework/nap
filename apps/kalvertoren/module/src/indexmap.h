#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <basetexture2d.h>
#include <nap/configure.h>
#include <color.h>
#include <pixmap.h>

namespace nap
{
	/**
	 * Type of image that contains a selection of colors that can be used as a lookup.
	 * This index map is always of a type RGB and stored uncompressed
	 */
	class NAPAPI IndexMap : public BaseTexture2D
	{
		RTTI_ENABLE(BaseTexture2D)
	public:
		// Color associated with an index
		using IndexColor = RGBColor8;

		virtual ~IndexMap();

		// Constructor
		IndexMap(const std::string& imgPath);

		// Default Constructor
		IndexMap() = default;

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the index color @index
		 * @param index number of the color, asserts when out of range
		 */
		const IndexColor& getColor(int index) const;

		/**
		 *	@return the total number of unique index colors
		 */
		int getCount() const;

		// Path to img on disk
		std::string				mImagePath;

		// Number of indexed colors inside the image
		int						mColorCount = 0;

	private:
		// Bitmap associated with this index map
		nap::Pixmap				mPixmap;

		// All the available indexed colors
		std::vector<IndexColor> mIndexColors;

		// Retrieves all the index colors from the map
		void findUniqueColors();
	};
}
