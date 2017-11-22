#pragma once

// Local Includes
#include "ledcolorpalette.h"
#include "indexmap.h"

// External Includes
#include <rtti/rttiobject.h>
#include <nap/objectptr.h>

namespace nap
{
	/**
	 * Holds all the available color palettes together with an index map
	 * This object makes sure that the all the available color palettes
	 * work with the assigned index map. The color palette must have the same
	 * or less amount of colors then the index map. Every color in the color palette
	 * must have a led color associated with it
	 */
	class LedColorContainer : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		virtual ~LedColorContainer();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		nap::ObjectPtr<IndexMap> mIndexMap;											///< The index map to use
		std::vector<nap::ObjectPtr<LedColorPalette>> mColorPalettes;				///< All the available color palettes
	};
}
