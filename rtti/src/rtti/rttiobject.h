#pragma once

// External Includes
#include <assert.h>
#include <memory>
#include <mutex>
#include <rtti/rtti.h>
#include <unordered_map>

namespace rtti
{
	/**
	 * The topmost base class for most NAP classes, providing hierarchical structure and inspectability which is useful
	 * for data serialization amongst other things.
	 * Instantiation should never be done "manually" but through the addChild methods. An object should always have a
	 * parent (except for the "world's" root Object, which belongs to Core)
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
