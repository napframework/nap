#pragma once

// External Includes
#include <rtti/rtti.h>
#include "utility/errorstate.h"
#include "utility/dllexport.h"

namespace nap
{
	namespace rtti
	{
		static const char* sIDPropertyName = "mID";

		/**
		 * The base class for all top-level objects that need to support serialization/deserialization.
		 * You only need to derive from this if your object should be serialized to the root of the document or needs to be
		 * able to be pointed to from other objects. If you're making, for example, a nested compound (i.e a plain struct), there is no need to derive from this class.
		 */
		class NAPAPI RTTIObject
		{
			RTTI_ENABLE()

		public:
			// Construction / Destruction
			RTTIObject();
			virtual ~RTTIObject();

			/**
			* Init this object after deserialization
			*/
			virtual bool init(utility::ErrorState& errorState) { return true; }

			/**
			 * Copy is not allowed
			 */
			RTTIObject(RTTIObject&) = delete;
			RTTIObject& operator=(const RTTIObject&) = delete;

			/**
			 * Move is not allowed
			 */
			RTTIObject(RTTIObject&&) = delete;
			RTTIObject& operator=(RTTIObject&&) = delete;

		public:
			static bool isIDProperty(rtti::Instance& object, const rtti::Property& property);
			std::string mID;
		};
	}
}
