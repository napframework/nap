/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <component.h>
#include <orbitcontroller.h>
#include <perspcameracomponent.h>
#include <componentptr.h>
#include <renderablemeshcomponent.h>
#include <box.h>

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

	public:
		ComponentPtr<OrbitController> mOrbitController;				///< Property: 'OrbitController' Camera orbit controller
		ComponentPtr<PerspCameraComponent> mCamera;					///< Property: 'Camera' Perspective Camera
		ComponentPtr<RenderableMeshComponent> mFlatRenderer;		///< Property: 'FlatRenderer' flat renderer
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
		bool init(utility::ErrorState& errorState) override;

		/**
		 * Bind and optionally frame mesh
		 * @param mesh the mesh to set
		 * @param frame if the camera should frame the mesh
		 * @param error contains the error if the mesh can't be set
		 */
		bool load(std::unique_ptr<IMesh>&& mesh, utility::ErrorState& error);

		/**
		 * Frames current selection
		 */
		void frame();

		ComponentInstancePtr<OrbitController> mOrbitController		= { this, &napkin::FrameMeshComponent::mOrbitController };
		ComponentInstancePtr<PerspCameraComponent> mCamera			= { this, &napkin::FrameMeshComponent::mCamera };
		ComponentInstancePtr<RenderableMeshComponent> mFlatRenderer = { this, &napkin::FrameMeshComponent::mFlatRenderer };

	private:
		std::unique_ptr<IMesh> mMesh = nullptr;
		math::Box mBounds;
		float mSpeedReference = 0.0f;
	};
}
