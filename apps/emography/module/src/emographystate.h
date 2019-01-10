#pragma once

// External Includes
#include <utility/dllexport.h>
#include <rtti/rtti.h>

namespace nap
{
	namespace emography
	{
		/**
		 * Represents the various stress related stimulation states
		 */
		enum class NAPAPI EState : int
		{
			Under	=  0,			///< Under stimulated
			Normal	=  1,			///< Normally stimulated
			Over	=  2,			///< Over stimulated
			Unknown = -1,			///< Unknown stimulated state
		};


		/**
		 * Represents an emography stress related intensity value.
		 * Simple struct like object that has only 1 field but is serializable
		 */
		class NAPAPI Intensity
		{
			RTTI_ENABLE()
		public:
			// Destructor
			virtual ~Intensity() { }

			/**
			 * Default constructor	
			 */
			Intensity()							{ }

			/**
			* Constructor
			* @param intensity stress intensity value
			*/
			Intensity(float intensity);

			/**
			 * @return if this is a valid intensity reading, ie: intensity value is >= 0
			 */
			bool isValid();

			float mIntensity = -1.0f;			///< Property: "Intensity" the stress related intensity value
		};
	}
}
