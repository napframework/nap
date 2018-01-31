#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <texture2dfromfile.h>
#include <nap/configure.h>
#include <color.h>
#include <bitmap.h>

namespace nap
{
	/**
	 * Type of image that contains a selection of colors that can be used as a lookup.
	 * This index map is always of a type RGB and stored uncompressed
	 */
	class NAPAPI IndexMap : public Texture2DFromFile
	{
		RTTI_ENABLE(Texture2DFromFile)
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
		 *	@return all the index colors
		 */
		const std::vector<IndexColor>& getColors() const				{ return mIndexColors; }

		/**
		 *	@return the total number of unique index colors
		 */
		int getCount() const;
		
		// Number of indexed colors inside the image
		int						mColorCount = 0;

	private:
		// All the available indexed colors
		std::vector<IndexColor> mIndexColors;

		// Retrieves all the index colors from the map
		void findUniqueColors();
	};
}
