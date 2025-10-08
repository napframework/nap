/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <component.h>

namespace napkin
{
	using namespace nap;
	class FrameMeshComponentInstance;

	/**
	 * Binds a mesh to a renderer and frames it in the viewport
	 */
	class FrameMeshComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(FrameMeshComponent, FrameMeshComponentInstance)
	};


	/**
	 * Binds a mesh to a renderer and frames it in the viewport
	 */
	class FrameMeshComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		FrameMeshComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize based on the resource
		 * @param errorState the error message when initialization fails
		 * @return if the component initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update framemeshcomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;
	};
}
