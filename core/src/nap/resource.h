#pragma once

#include <rtti/rttiobject.h>
#include <utility/dllexport.h>

namespace nap
{
	/**
	 * A resource is a stand-alone, static object that can be authored in json.
	 * Derive from this class to create your own resource type and
	 * implement the init() call to initialize the object after de-serialization.
	 */
	class NAPAPI Resource : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		Resource();

		/**
		 * Override in derived classes to initialize resource after de-serialization
		 */
		using rtti::RTTIObject::init;
	};
}
