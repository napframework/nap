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
#include <renderable2dtextcomponent.h>

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
		ComponentPtr<TransformComponent> mBBoxTransform;			///< Property: 'BBoxTransform' bounding box transform
		ComponentPtr<RenderableMeshComponent> mBBoxRenderer;		///< Property: 'BBoxRenderer' bounding box renderer
		ComponentPtr<Renderable2DTextComponent> mBBoxTextRenderer;	///< Property: 'BBoxTextRenderer' bounding box text renderer
	};


	/**
	 * Binds a mesh to a renderer and frames it in the viewport
	 */
	class FrameMeshComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		FrameMeshComponentInstance(EntityInstance& entity, Component& resource);

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

		/**
		 * If a mesh has been loaded
		 */
		bool loaded() const { return mMesh != nullptr; }

		/**
		 * @return loaded mesh, nullptr if no mesh is loaded
		 */
		nap::IMesh* getMesh() const { return mMesh.get(); }

		/**
		 * @return if the component has a mesh assigned
		 */
		bool hasMesh() const { return mMesh != nullptr; }

		/**
		 * @return mesh bounds
		 */
		const math::Box& getBounds() const { return mBounds; }

		/**
		 * Set blend mode
		 */
		void setBlendMode(EBlendMode mode) { mBlendMode = mode; }

		/**
		 * Set wireframe width
		 */
		void setLineWidth(float size);

		/**
		 * Set mesh color
		 */
		void setMeshColor(const RGBAColorFloat& color) { mMeshColor = color; }

		/**
		 * @return mesh color
		 */
		const RGBAColorFloat& getMeshColor() const { return mMeshColor; }

		/**
		 * Set mesh color
		 */
		void setWireColor(const RGBAColorFloat& color) { mWireColor = color; }

		/**
		 * @return mesh color
		 */
		const RGBAColorFloat& getWireColor() const { return mWireColor; }

		/**
		 * Set bounds color
		 */
		void setBoundsColor(const RGBColorFloat& color) { mBBoxColor = color; }

		/**
		 * @return bounds color
		 */
		const RGBColorFloat& getBoundsColor() const { return mBBoxColor; }

		/**
		 * @return current blend mode
		 */
		EBlendMode getBlendMode() const { return mBlendMode; }

		/**
		 * @return loaded mesh topology
		 */
		EDrawMode getTopology() const { return mTopology; }

		/**
		 * Draws selection as mesh
		 */
		void drawMesh();

		/**
		 * Draws wireframe overlay if possible
		 */
		void drawWireframe();

		/**
		 * @return if the current selection has a wireframe
		 */
		bool hasWireframe() const;

		/**
		 * Draws bbox frame mesh & min, max coordinates
		 */
		void drawBounds();

		ComponentInstancePtr<OrbitController> mOrbitController				= { this, &napkin::FrameMeshComponent::mOrbitController };
		ComponentInstancePtr<PerspCameraComponent> mCamera					= { this, &napkin::FrameMeshComponent::mCamera };
		ComponentInstancePtr<RenderableMeshComponent> mFlatRenderer			= { this, &napkin::FrameMeshComponent::mFlatRenderer };
		ComponentInstancePtr<TransformComponent> mBBoxTransform				= { this, &napkin::FrameMeshComponent::mBBoxTransform };
		ComponentInstancePtr<RenderableMeshComponent> mBBoxRenderer			= { this, &napkin::FrameMeshComponent::mBBoxRenderer };
		ComponentInstancePtr<Renderable2DTextComponent> mBBoxTextRenderer	= { this, &napkin::FrameMeshComponent::mBBoxTextRenderer };

	private:
		std::unique_ptr<IMesh> mMesh = nullptr;
		math::Box mBounds;
		float mSpeedReference = 0.0f;
		EPolygonMode mPolyMode = EPolygonMode::Fill;
		EDrawMode mTopology = EDrawMode::Unknown;

		// Uniforms
		UniformVec3Instance* mMeshColorUniform = nullptr;
		UniformFloatInstance* mMeshAlphaUniform = nullptr;
		UniformVec3Instance* mBBoxColorUniform = nullptr;
		RenderService* mRenderService = nullptr;

		// Blend-mode
		EBlendMode mBlendMode = EBlendMode::Opaque;
		RGBAColorFloat mMeshColor = { 0.682352960, 0.674509823, 0.643137276, 1.0f};
		RGBAColorFloat mWireColor = {0.0f, 0.0f, 0.0f, 0.2f};
		RGBColorFloat mBBoxColor = { 1.0f, 1.0f, 1.0f };

		void draw(const RGBAColorFloat& color);
	};
}
