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
	using LayerIndex = int;

	// Forward declares
	class RenderChain;

	/**
	 * Render layers can be used to group render components and configure the order in which they should be rendered.
	 * `nap::RenderLayer` objects are ordered in a `nap::RenderChain` under the property `Layers`. The index
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
		friend class RenderChain;
		RTTI_ENABLE(Resource)
	public:
		RenderLayer() = default;
		virtual ~RenderLayer() = default;

		/**
		 * Returns the layer index, set by the `RenderChain` on initialization.
		 * Do not call this function on initialization of your resource! The index might not have been assigned yet.
		 * 
		 * @return the priority index of the layer in the registry.
		 */
		LayerIndex getIndex() const				{ assert(mIndex >= 0); return mIndex; }

		/**
		 * @return the name of the layer in the registry
		 */
		const std::string& getName() const		{ return mName; }

		std::string mName;						///< Property: 'Name' The render layer name

	private:
		LayerIndex mIndex = -1;					///< The index is set on initialization of a registry
	};
	
	using RenderLayerList = std::vector<rtti::ObjectPtr<RenderLayer>>;

	
	/**
	 * Configures and controls the order in which components are rendered.
	 * 
	 * `nap::RenderLayer` objects are ordered in a `nap::RenderChain`, under the property `Layers`. The index
	 * of the layer in this list determines the rank of the `nap::RenderLayer` where 0 is the front, and the last
	 * index is the back. Render components without a layer assigned default to the front (index 0).
	 *
	 * Don't include more than one `nap::RenderChain` in your application, this could lead to undefined behavior.
	 */
	class NAPAPI RenderChain : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		RenderChain() = default;
		virtual ~RenderChain() = default;

		/**
		 * Assigns layer indices and ensures entries are valid.
		 * @param errorState contains the error if initialization fails
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		RenderLayerList mLayers;						///< Property: 'Layers' The render layer list in order
	};
}
