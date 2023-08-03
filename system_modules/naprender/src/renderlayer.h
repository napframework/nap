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
	using LayerIndex = uint;

	// Forward declares
	class RenderLayerRegistry;

	/**
	 * Render layer
	 */
	class NAPAPI RenderLayer : public Resource
	{
		friend class RenderLayerRegistry;
		RTTI_ENABLE(Resource)
	public:
		RenderLayer() = default;
		virtual ~RenderLayer() = default;

		/**
		 * @return the index of the layer in the registry.
		 */
		uint getIndex() const					{ return mIndex; }

		/**
		 * @return the name of the layer in the registry
		 */
		const std::string& getName() const		{ return mName; }

		std::string mName;								///< Property: 'Name' The layer name
		ResourcePtr<RenderLayerRegistry> mRegistry;		///< Property: 'Registry' The registry

	private:
		LayerIndex mIndex = 0U;			// The index is set on initialization of a registry
	};
	
	using RenderLayerList = std::vector<rtti::ObjectPtr<RenderLayer>>;

	
	/**
	 * List of render layers
	 */
	class NAPAPI RenderLayerRegistry : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		RenderLayerRegistry() = default;
		virtual ~RenderLayerRegistry() = default;

		virtual bool init(utility::ErrorState& errorState) override;

		RenderLayerList mLayers;			///< Property: 'Layers' The layer list in order
	};
}
