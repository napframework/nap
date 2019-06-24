#pragma once
#include "mesh.h"

#include "triangleiterator.h"

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <flexblockmesh.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * FrameMesh
	 * Renders the frame of a flexblock object
	 */
	class NAPAPI FrameMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		virtual bool init(utility::ErrorState& errorState) override;

		void setControlPoints(std::vector<glm::vec3> controlPoints);

		virtual MeshInstance& getMeshInstance() override { return *mMeshInstance; }

		virtual const MeshInstance& getMeshInstance() const override { return *mMeshInstance; }
		ResourcePtr<FlexBlockMesh> mFlexBlockMesh = nullptr;							///< Property: 'ReferenceMesh' link to the mesh that is used as a reference, can be null (ie: nothing)
		
		const math::Box& getBox() const { return mBox; }

		void setFramePoints(std::vector<glm::vec3> frame);
	public:
		glm::vec3 mSize = { 1.0f, 1.0f, 1.0f };			///< Property: 'Dimensions' of the frame
	protected:
		// Creates the mesh and shape
		bool createMeshInstance(nap::utility::ErrorState& error);

		// Sets up the mesh instance based on the total amount of points to scatter
		bool setup(nap::utility::ErrorState& error);

		// The actual mesh that contains all the points
		std::unique_ptr<MeshInstance> mMeshInstance = nullptr;

		// Position Attribute data
		nap::Vec3VertexAttribute* mPositionAttr = nullptr;

		// Color attribute data
		std::vector<nap::Vec4VertexAttribute*> mColorAttrs;

		math::Box mBox = { 1.0f, 1.0f, 1.0f };
	};
}
