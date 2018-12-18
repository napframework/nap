#pragma once

// External Includes
#include<rtti/objectptr.h>

namespace nap
{
	/**
	 * Can be used to point to other resources in a json file
	 * This is just a regular object ptr but allows for a more explicit definition of a link
	 */
	template<typename T>
	using ResourcePtr = rtti::ObjectPtr<T>;
}