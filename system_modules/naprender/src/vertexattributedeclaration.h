/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <string>
#include <unordered_map>
#include "vulkan/vulkan_core.h"
#include "rtti/typeinfo.h"

namespace nap
{
	/**
	* Represents an Vulkan vertex shader attribute
	*/
	class VertexAttributeDeclaration
	{
	public:
		// Constructor
		VertexAttributeDeclaration(const std::string& name, int location, int elementSize, VkFormat format);
		VertexAttributeDeclaration() = delete;

		std::string		mName;							///< Name of the shader attribute
		int				mLocation;
		int				mElementSize;					///< Element size of this vertex
		VkFormat		mFormat;
	};

	using VertexAttributeDeclarations = std::unordered_map<std::string, std::unique_ptr<VertexAttributeDeclaration>>;
}

//////////////////////////////////////////////////////////////////////////
// Hash
//////////////////////////////////////////////////////////////////////////
namespace std
{
	template<>
	struct hash<nap::VertexAttributeDeclaration>
	{
		size_t operator()(const nap::VertexAttributeDeclaration &k) const
		{
			return hash<std::string>()(k.mName);
		}
	};
}
