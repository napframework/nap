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
#include <renderadvancedservice.h>
#include <rotatecomponent.h>

namespace napkin
{
	using namespace nap;
	class MeshPreviewLoadComponentInstance;

	/**
	 * Binds a mesh to a renderer and frames it in the viewport
	 */
	class MeshPreviewLoadComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(MeshPreviewLoadComponent, MeshPreviewLoadComponentInstance)

	public:
		ComponentPtr<OrbitController> mOrbitController;				///< Property: 'OrbitController' Camera orbit controller
		ComponentPtr<PerspCameraComponent> mCamera;					///< Property: 'Camera' Perspective Camera
		ComponentPtr<RenderableMeshComponent> mFlatRenderer;		///< Property: 'FlatRenderer' flat renderer
		ComponentPtr<TransformComponent> mBBoxTransform;			///< Property: 'BBoxTransform' bounding box transform
		ComponentPtr<RenderableMeshComponent> mBBoxRenderer;		///< Property: 'BBoxRenderer' bounding box renderer
		ComponentPtr<Renderable2DTextComponent> mBBoxTextRenderer;	///< Property: 'BBoxTextRenderer' bounding box text renderer
		ComponentPtr<RenderableMeshComponent> mShadedRenderer;		///< Property: 'ShadedRenderer' shaded light renderer
		ComponentPtr<TransformComponent> mObjectTransform;			///< Property: 'MeshTransform' global mesh render xform
		ComponentPtr<RotateComponent> mRotator;						///< Property: 'Rotator' mesh rotate component
		ComponentPtr<RenderableMeshComponent> mWireRenderer;		///< Property: 'WireRenderer' wire render component
		ComponentPtr<TransformComponent> mWorldTransform;			///< Property: 'RenderTransform' final render transform 
	};


	/**
	 * Binds a mesh to a renderer and frames it in the viewport
	 */
	class MeshPreviewLoadComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		MeshPreviewLoadComponentInstance(EntityInstance& entity, Component& resource);

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
		const math::Box& getObjectBounds() const { return mObjectBounds; }

		/**
		 * Set blend mode
		 */
		void setBlendMode(EBlendMode mode) { mBlendMode = mode; }

		/**
		 * Get current blend mode
		 */
		EBlendMode getBlendMode() const { return mBlendMode; }

		/**
		 * Set wireframe width
		 */
		void setWireWidth(float width);

		/**
		 * @return wireframe line width
		 */
		float getWireWidth() const { return mWireWidth; }

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
		 * @return loaded mesh topology
		 */
		EDrawMode getTopology() const { return mTopology; }

		/**
		 * @return if wireframe is drawn on top of mesh
		 */
		bool getDrawWireframe() const { return mDrawWireframe; }

		/**
		 * Set if wireframe is drawn on top of mesh
		 */
		void setDrawWireFrame(bool draw) { mDrawWireframe = draw; }

		/**
		 * @return if bounding box is drawn
		 */
		bool getDrawBounds() const { return mDrawBounds; }

		/**
		 * Set if bounding box is drawn
		 */
		void setDrawBounds(bool draw) { mDrawBounds = draw; }

		/**
		 * Set rotate speed in seconds
		 */
		void setRotateSpeed(float speed) { mRotator->setSpeed(speed); }

		/**
		 * @return mesh rotate speed in seconds
		 */
		float getRotateSpeed() { return mRotator->getSpeed(); }

		/**
		 * Set mesh scale
		 */
		void setScale(const glm::vec3& scale) { mWorldTransform->setScale(scale); }

		/**
		 * @return current mesh scale
		 */
		const glm::vec3& getScale() { return mWorldTransform->getScale(); }

		/**
		 * Set mesh rotate per axis in degrees
		 */
		void setRotate(const glm::vec3& eulerAngles);

		/**
		 * @return rotation per axis in degrees
		 */
		const glm::vec3& getRotate() const { return mRotate; }

		/**
		 * @return if a triangle mesh is loaded
		 */
		bool isTriangleMesh() const;

		/**
		 * @reutn number of triangles in the mesh, -1 if mesh has no triangles
		 */
		int getTriangleCount() const;

		/**
		 * Draws mesh, wire-frame and bounds
		 */
		void draw();

		/**
		 * Draws selection as mesh
		 */
		void drawMesh();

		/**
		 * Draws wire-frame overlay
		 */
		void drawWireframe();

		/**
		 * @return if the current selection has a wire-frame
		 */
		bool hasWireframe() const;

		/**
		 * Draws bbox frame mesh & min, max coordinates
		 */
		void drawBounds();

		/**
		 * Clear current selection
		 */
		void clear();

		ComponentInstancePtr<OrbitController> mOrbitController				= { this, &napkin::MeshPreviewLoadComponent::mOrbitController };
		ComponentInstancePtr<PerspCameraComponent> mCamera					= { this, &napkin::MeshPreviewLoadComponent::mCamera };
		ComponentInstancePtr<RenderableMeshComponent> mFlatRenderer			= { this, &napkin::MeshPreviewLoadComponent::mFlatRenderer };
		ComponentInstancePtr<TransformComponent> mBBoxTransform				= { this, &napkin::MeshPreviewLoadComponent::mBBoxTransform };
		ComponentInstancePtr<RenderableMeshComponent> mBBoxRenderer			= { this, &napkin::MeshPreviewLoadComponent::mBBoxRenderer };
		ComponentInstancePtr<Renderable2DTextComponent> mBBoxTextRenderer	= { this, &napkin::MeshPreviewLoadComponent::mBBoxTextRenderer };
		ComponentInstancePtr<RenderableMeshComponent> mShadedRenderer		= { this, &napkin::MeshPreviewLoadComponent::mShadedRenderer };
		ComponentInstancePtr<TransformComponent> mObjectTransform			= { this, &napkin::MeshPreviewLoadComponent::mObjectTransform };
		ComponentInstancePtr<RotateComponent> mRotator						= { this, &napkin::MeshPreviewLoadComponent::mRotator };
		ComponentInstancePtr<RenderableMeshComponent> mWireRenderer			= { this, &napkin::MeshPreviewLoadComponent::mWireRenderer };
		ComponentInstancePtr<TransformComponent> mWorldTransform			= { this, &napkin::MeshPreviewLoadComponent::mWorldTransform };

	private:
		std::unique_ptr<IMesh> mMesh = nullptr;
		math::Box mObjectBounds;
		math::Box mWorldBounds;
		float mSpeedReference = 0.0f;
		float mWireWidth = 1.0f;
		EPolygonMode mPolyMode = EPolygonMode::Fill;
		EDrawMode mTopology = EDrawMode::Unknown;
		glm::vec3 mRotate = { 0.0f, 0.0f, 0.0f };

		// Uniforms
		UniformVec3Instance* mFlatColorUniform = nullptr;
		UniformFloatInstance* mFlatAlphaUniform = nullptr;
		UniformVec3Instance* mShadedDiffuseUniform = nullptr;
		UniformFloatInstance* mShadedAlphaUniform = nullptr;
		UniformVec3Instance* mBBoxColorUniform = nullptr;
		UniformVec3Instance* mWireColorUniform = nullptr;
		UniformFloatInstance* mWireAlphaUniform = nullptr;
		UniformFloatInstance* mWireDisplacementUniform = nullptr;
		RenderService* mRenderService = nullptr;

		// Colors
		EBlendMode mBlendMode = EBlendMode::Additive;
		RGBAColorFloat mMeshColor = { 0.682352960, 0.674509823, 0.643137276, 0.45f};
		RGBAColorFloat mWireColor = {1.0f, 1.0f, 1.0f, 0.098f};
		RGBColorFloat mBBoxColor = { 1.0f, 1.0f, 1.0f };

		// Service
		RenderAdvancedService* mRenderAdvancedService = nullptr;
		RenderableMesh mShadedRenderMesh;
		RenderableMesh mWireRenderMesh;
		bool mDrawWireframe = false;
		bool mDrawBounds = true;
	};
}
