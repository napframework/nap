#pragma once

#include <rtti/object.h>
#include <utility/dllexport.h>

namespace nap
{
	/**
	 * A resource is a stand-alone, static object that can be authored in json.
	 * Derive from this class to create your own resource type and
	 * implement the init() call to initialize the object after de-serialization.
	 */
	class NAPAPI Resource : public rtti::Object
	{
		RTTI_ENABLE(rtti::Object)
	public:
		Resource();

		/**
		 * Override in derived classes to initialize resource after de-serialization
		 */
		using rtti::Object::init;
	};
}
