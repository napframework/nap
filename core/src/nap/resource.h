#pragma once

// Local Includes
#include "core.h"

// External Includes
#include <string>

namespace nap
{
	/**
	* Abstract base class for any Asset. Could be a TextureAsset, ModelAsset or AudioAsset for example.
	*/
	class Resource
	{
		RTTI_ENABLE()
	public:
		Resource() = default;
		/**
		* @return Human readable string representation of this path
		*/
		virtual const std::string& getDisplayName() const = 0;

		/**
		* Provided a core to work with, create an instance of this resource as a child of the provided parent.
		* @param core The core, necessary for some client operations
		* @param parent The parent of which the newly created Object will be a child
		* @return The newly created Object
		*/
		virtual Object* createInstance(Core& core, Object& parent) = 0;
	};
}

RTTI_DECLARE_BASE(nap::Resource)
