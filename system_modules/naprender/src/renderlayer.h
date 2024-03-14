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
#include <string>
#include <vector>

namespace nap
{
	// Forward declares
	class Core;
	class RenderService;

	namespace layer
	{
		namespace rank
		{
			constexpr int invalid = -1;		///< Layer not registered
		}
	}

	/**
	 * Render layers can be used to group render components and configure the order in which they should be rendered.
	 * `nap::RenderLayer` objects are ordered in a `nap::RenderChain` under the property `Layers`. The index
	 * of the layer in this list determines the rank of the `nap::RenderLayer` where 0 is the front, and the last
	 * index is the back. Render components without a layer assigned default to the front (index 0).
	 *
	 * One useful approach for layers is when rendering a sky box or some other object that fills the background. This
	 * should always be rendered first, regardless of the transform it has. To do this, you add a layer named
	 * "Background" to the `nap::RenderChain` and assign it to the background render component. Other objects
	 * in the scene can be assigned to the layer "Default" (on index 0) and will be sorted routinely based on the
	 * specified sorting algorithm.
	 */
	class NAPAPI RenderLayer : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * @return the name of the layer in the chain
		 */
		const std::string& getName() const		{ return mName; }

		std::string mName;						///< Property: 'Name' The render layer name

	private:
		RenderService* mRenderService = nullptr;
	};

	
	/**
	 * Controls the order in which components are rendered.
	 *
	 * The index of a layer in the chain = the rank that ultimately controls the order in which components are rendered.
	 * Components with a layer of rank (index) 0 are rendered first, after that components with a rank (index) of 1 etc.
	 * Note that layers are optional, components without a layer are assigned a default rank of 0.
	 */
	class NAPAPI RenderChain : public Resource
	{
		friend class RenderService;
		RTTI_ENABLE(Resource)
	public:
		RenderChain(nap::Core& core);

		// Invalid rank
		static constexpr int invalidRank = -1;

		/**
		 * Assigns layer indices, ensures entries are valid and registers the chain for render operations
		 * @param errorState contains the error if initialization fails
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Unregisters the chain for subsequent render operations
		 */
		virtual void onDestroy() override;

		std::vector<rtti::ObjectPtr<RenderLayer>> mLayers;			///< Property: 'Layers' The render layers in ranked order

	private:
		RenderService* mRenderService = nullptr;
		std::unordered_map<const RenderLayer*, int> mRankMap;
	};
}
