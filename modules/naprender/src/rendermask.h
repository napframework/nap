/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "vulkan/vulkan_core.h"
#include "rtti/typeinfo.h"

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <nap/numeric.h>
#include <string>
#include <vector>

namespace nap
{
	// RenderMask supports up to 64 different tags
	using RenderMask = uint64;

	/**
	 * Render tag.
	 */
	class NAPAPI RenderTag : public Resource
	{
		friend class RenderTagRegistry;
		RTTI_ENABLE(Resource)
	public:
		RenderTag() = default;
		virtual ~RenderTag() = default;

		/**
		 * @return the index of the tag in the registry.
		 */
		uint getIndex() const					{ return mIndex; }

		std::string mName;									///< Property: 'Name' The tag name

	private:
		uint8 mIndex = 0U;
	};
	
	using RenderTagList = std::vector<rtti::ObjectPtr<RenderTag>>;

	
	/**
	 * List of render tags.
	 */
	class NAPAPI RenderTagRegistry : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		RenderTagRegistry() = default;
		virtual ~RenderTagRegistry() = default;

		virtual bool init(utility::ErrorState& errorState) override;

		RenderTagList mTags;			///< Property: 'Tags' The tag list
	};

	/**
	 * Creates a render mask from a list of tags
	 * @param tags
	 * @return the render mask
	 */
	static RenderMask createRenderMask(const RenderTagList& tags)
	{
		RenderMask mask = 0U;
		for (const auto& tag : tags)
			mask |= 0x01 << tag->getIndex();
		return mask;
	}


	/**
	 * Compares component and inclusion masks
	 * @param componentMask
	 * @param inclusionMask
	 * @return true if the componentMask is included in the inclusionMask
	 */
	static bool compareRenderMask(RenderMask componentMask, RenderMask inclusionMask)
	{
		return (componentMask == 0U) || ((componentMask & inclusionMask) > 0U);
	}
}
