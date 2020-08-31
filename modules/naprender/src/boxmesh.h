#pragma once

// Local Includes
#include "mesh.h"

// External Includes
#include <box.h>

namespace nap
{
	class RenderService;

	/**
	 * Predefined box mesh with additional uv, color and normal vertex attributes.
	 * The UV coordinates are always 0-1. The box consists of 6 planes.
	 * The vertices of the individual planes are not shared.
	 */
	class NAPAPI BoxMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:

		BoxMesh(Core& core);

		/**
		 * Sets up and initializes the box as a mesh based on the provided parameters.
		 * @param errorState contains the error message if the mesh could not be created.
		 * @return if the mesh was successfully created and initialized.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Creates and prepares the mesh but doesn't initialize it.
		 * Call this when you want to prepare a box without creating the GPU representation.
		 * You have to manually call init() on the mesh instance afterwards.
		 */
		void setup();

		/**
		 * @return the mesh instance that can be rendered to screen
		 */
		virtual MeshInstance& getMeshInstance() override { return *mMeshInstance; }

		/**
		 * @return the mesh instance that can be rendered to screen
		 */
		virtual const MeshInstance& getMeshInstance() const override { return *mMeshInstance; }

		/**
		 * @return the box that wraps the mesh
		 */
		const math::Box& getBox() const { return mBox; }

	public:
		glm::vec3		mSize		= { 1.0f, 1.0f, 1.0f };		///< Property: 'Dimensions' of the box
		glm::vec3		mPosition	= { 0.0f, 0.0f, 0.0f };		///< Property: 'Position'  of the box
		EMeshDataUsage	mUsage = EMeshDataUsage::Static;		///< Property: 'Usage' If the mesh is uploaded once or frequently updated.
		ECullMode		mCullMode = ECullMode::Back;			///< Property: 'CullMode' controls which triangles are culled, back facing, front facing etc.

	protected:
		/**
		 * Constructs the mesh based on the given dimensions
		 * @param box the box to build the mesh from
		 * @param mesh the mesh that is constructed
		 */
		void constructBox(const math::Box& box, nap::MeshInstance& mesh);

	private:
		RenderService* mRenderService;
		std::unique_ptr<MeshInstance> mMeshInstance;
		math::Box mBox = { 1.0f, 1.0f, 1.0f };
	};
}
