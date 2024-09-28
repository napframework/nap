/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/device.h>
#include <nap/resourceptr.h>
#include <nap/numeric.h>
#include <nap/group.h>
#include <rtti/typeinfo.h>
#include <string>
#include <vector>

namespace nap
{
	// Forward declares
	class RenderService;
	class Core;

	// RenderMask definition, supports up to 64 different tags
	using RenderMask = uint64;

	namespace mask
	{
		// Includes every tag
		constexpr RenderMask all  = std::numeric_limits<RenderMask>::max();

		// No tag provided
		constexpr RenderMask none = 0;
	}

	/**
	 * Render tags can be used to categorize render components. Unlike render layers, tags are unordered and multiple
	 * of them can be assigned to a single render component. Each tag resource registers itself in the render service
	 * and is assigned a unique tag index on app initialization. This ensures tags can be composited into render masks,
	 * which are bit flags that are fast to compare.
	 *
	 * One useful example would be to categorize specific components as "Debug", distinguishing objects used as visual
	 * aid for debugging purposes from standard objects (tag "Default"). They may be excluded from rendering based on
	 * settings or a window setup for instance. You could do the following:
	 *
	 * `````{.cpp}
	 * // Consider caching the render mask
	 * auto debug_tag = mResourceManager->findObject("DebugTag");
	 * auto scene_tag = mResourceManager->findObject("SceneTag");
	 * mRenderService->renderObjects(renderTarget, camera, render_comps, debug_tag | scene_tag);
	 * `````
	 */
	class NAPAPI RenderTag : public Device
	{
		friend class RenderService;
		RTTI_ENABLE(Device)
	public:
		RenderTag(Core& core);
		virtual ~RenderTag() = default;

		/**
		 * Register the RenderTag with the RenderService
		 */
		virtual bool start(utility::ErrorState& errorState) override;

		/**
		 * Unregister the RenderTag with the RenderService
		 */
		virtual void stop() override;

		/**
		 * Returns the mask assigned by the renderer to this tag.
		 * The mask can be composited together with masks of other tags to create a unique bit mask:
		 *
		 * `````{.cpp}
		 * auto debug_mask = mResourceManager->findObject("DebugTag").getMask();
		 * auto scene_mask = mResourceManager->findObject("SceneTag").getMask();
		 * mRenderService->renderObjects(renderTarget, camera, render_comps, debug_mask | scene_mask);
		 * `````
		 *
		 * @return the mask assigned to this tag
		 */
		RenderMask getMask() const;

		/**
		 * Mask inclusion operator: Combines this tag with the given tag.
		 *
		 * `````{.cpp}
		 * auto debug_tag = *mResourceManager->findObject("DebugTag");
		 * auto scene_tag = *mResourceManager->findObject("SceneTag");
		 * mRenderService->renderObjects(renderTarget, camera, render_comps, debug_tag | scene_tag);
		 * `````
		 *
		 * @param other the tag to composite
		 * @return the composited mask
		 */
		RenderMask operator|(const RenderTag& other) const		{ return this->getMask() | other.getMask(); }

		/**
		 * Mask inclusion operator: Combines this tag with the given mask.
		 * @param other the mask to composite
		 * @return the composited mask
		 */
		RenderMask operator|(RenderMask other) const			{ return other | this->getMask(); }

		/**
		 * @return The mask assigned to this tag
		 */
		operator RenderMask() const								{ return this->getMask(); }

		std::string mName;										///< Property: 'Name' The tag name

	private:
		RenderService& mRenderService;
	};

	// RenderTagGroup definition
	using RenderTagGroup = Group<RenderTag>;
}
