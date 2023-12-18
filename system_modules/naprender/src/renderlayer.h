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
	 * Render layers can be used to group render components and configure the order in which they should be rendered.
	 * `nap::RenderLayer` objects are ordered in a `nap::RenderLayerRegistry` under the property `Layers`. The index
	 * of the layer in this list determines the rank of the `nap::RenderLayer` where 0 is the front, and the last
	 * index is the back. Render components without a layer assigned default to the front (index 0).
	 *
	 * One useful approach for layers is when rendering a sky box or some other object that fills the background. This
	 * should always be rendered first, regardless of the transform it has. To do this, you can create a layer named
	 * "Background" on the last index in the registry and assign it to the background render component. Other objects
	 * in the scene can be assigned to the layer "Default" on index 0 and will be sorted routinely based on the
	 * specified sorting algorithm.
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

		std::string mName;								///< Property: 'Name' The render layer name

	private:
		LayerIndex mIndex = 0U;			// The index is set on initialization of a registry
	};
	
	using RenderLayerList = std::vector<rtti::ObjectPtr<RenderLayer>>;

	
	/**
	 * A Render layer registry defines a set of render layers and their order. Each render component that has a layer
	 * assigned in the property `Layer` must also assign its parent registry in the property `LayerRegistry`. This is
	 * to ensure the layer registry resource is always initialized before the layer resources. Including more than one
	 * `nap::LayerRegistry` is not allowed and can cause undefined behavior.
	 * 
	 * Render layers can be used to group render components and configure the order in which they should be rendered.
	 * `nap::RenderLayer` objects are ordered in a `nap::RenderLayerRegistry` under the property `Layers`. The index
	 * of the layer in this list determines the rank of the `nap::RenderLayer` where 0 is the front, and the last
	 * index is the back. Render components without a layer assigned default to the front (index 0).
	 */
	class NAPAPI RenderLayerRegistry : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		RenderLayerRegistry() = default;
		virtual ~RenderLayerRegistry() = default;

		virtual bool init(utility::ErrorState& errorState) override;

		RenderLayerList mLayers;						///< Property: 'Layers' The render layer list in order
	};
}
