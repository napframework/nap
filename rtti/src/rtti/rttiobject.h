#pragma once

// External Includes
#include <assert.h>
#include <memory>
#include <mutex>
#include <rtti/rtti.h>
#include <unordered_map>

namespace nap
{
	namespace rtti
	{
		/**
		 * The base class for all top-level objects that need to support serialization/deserialization.
		 * You only need to derive from this if your object should be serialized to the root of the document or needs to be
		 * able to be pointed to from other objects. If you're making, for example, a nested compound (i.e a plain struct), there is no need to derive from this class.
		 */
		class RTTIObject
		{
			RTTI_ENABLE()

		public:
			// Construction / Destruction
			RTTIObject();
			virtual ~RTTIObject() = default;

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
			std::string mID;
		};
	}
}
