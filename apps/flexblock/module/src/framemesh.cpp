// Local Includes
#include "FrameMesh.h"
#include "meshutils.h"

// External Includes
#include <mathutils.h>
#include <glm/geometric.hpp>

RTTI_BEGIN_CLASS(nap::FrameMesh)
	RTTI_PROPERTY("Size", &nap::FrameMesh::mSize, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ReferenceMesh", &nap::FrameMesh::mFlexBlockMesh, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool FrameMesh::init(utility::ErrorState& errorState)
	{
		// Make sure the reference mesh contains triangles
		bool contains_triangles = false;
		for (int i = 0; i < mFlexBlockMesh->getMeshInstance().getNumShapes(); i++)
		{
			if (utility::isTriangleMesh(mFlexBlockMesh->getMeshInstance().getShape(i)))
				contains_triangles = true;
		}

		if (!errorState.check(contains_triangles, "reference mesh doesn't contain any triangles: %s", this->mID.c_str()))
			return false;

		// Create mesh instance
		if (!createMeshInstance(errorState))
			return false;

		// Setup instance (allocate vertex attributes)
		if (!setup(errorState))
			return false;

		return true;
	}


	bool FrameMesh::createMeshInstance(nap::utility::ErrorState& error)
	{
		if (!error.check(mMeshInstance == nullptr, "unable to create new mesh, already assigned: %s", this->mID.c_str()))
			return false;

		// There is only 1 shape associated with the scatter mesh
		mMeshInstance = std::make_unique<MeshInstance>();
		mMeshInstance->createShape();

		return true;
	}

	void FrameMesh::setControlPoints(std::vector<glm::vec3> controlPoints)
	{
		auto verts = mPositionAttr->getData();
		verts[1] = controlPoints[0];
		verts[3] = controlPoints[1];
		verts[5] = controlPoints[2];
		verts[7] = controlPoints[3];
		verts[9] = controlPoints[4];
		verts[11] = controlPoints[5];
		verts[13] = controlPoints[6];
		verts[15] = controlPoints[7];
		mPositionAttr->setData(verts);

		utility::ErrorState error;
		mMeshInstance->update(error);
	}

	void FrameMesh::setFramePoints(std::vector<glm::vec3> frame)
	{
		auto box = mFlexBlockMesh->getBox();

		const glm::vec3& minBox = box.getMin();
		const glm::vec3& maxBox = box.getMax();
		glm::vec3 minFrame(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
		glm::vec3 maxFrame(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

		for (int i = 0; i < frame.size(); i++)
		{
			if (frame[i].x < minFrame.x)
			{
				minFrame.x = frame[i].x;
			}
			if (frame[i].y < minFrame.y)
			{
				minFrame.y = frame[i].y;
			}
			if (frame[i].z < minFrame.z)
			{
				minFrame.z = frame[i].z;
			}

			if (frame[i].x > maxFrame.x)
			{
				maxFrame.x = frame[i].x;
			}
			if (frame[i].y > maxFrame.y)
			{
				maxFrame.y = frame[i].y;
			}
			if (frame[i].z > maxFrame.z)
			{
				maxFrame.z = frame[i].z;
			}
		}

		auto & verts = mPositionAttr->getData();
		verts.clear();

		// control point 1
		verts.push_back(glm::vec3(minFrame.x, minFrame.y, maxFrame.z));
		verts.push_back(glm::vec3(minBox.x, minBox.y, maxBox.z));

		// control point 2
		verts.push_back(glm::vec3(maxFrame.x, minFrame.y, maxFrame.z));
		verts.push_back(glm::vec3(maxBox.x, minBox.y, maxBox.z));

		// control point 3
		verts.push_back(glm::vec3(minFrame.x, maxFrame.y, maxFrame.z));
		verts.push_back(glm::vec3(minBox.x, maxBox.y, maxBox.z));

		// control point 4
		verts.push_back(glm::vec3(maxFrame.x, maxFrame.y, maxFrame.z));
		verts.push_back(glm::vec3(maxBox.x, maxBox.y, maxBox.z));

		// control point 5
		verts.push_back(glm::vec3(maxFrame.x, minFrame.y, minFrame.z));
		verts.push_back(glm::vec3(maxBox.x, minBox.y, minBox.z));

		// control point 6
		verts.push_back(glm::vec3(minFrame.x, minFrame.y, minFrame.z));
		verts.push_back(glm::vec3(minBox.x, minBox.y, minBox.z));

		// control point 7
		verts.push_back(glm::vec3(maxFrame.x, maxFrame.y, minFrame.z));
		verts.push_back(glm::vec3(maxBox.x, maxBox.y, minBox.z));

		// control point 8
		verts.push_back(glm::vec3(minFrame.x, maxFrame.y, minFrame.z));
		verts.push_back(glm::vec3(minBox.x, maxBox.y, minBox.z));

		// construct frame
		verts.push_back(glm::vec3(minFrame.x, minFrame.y, minFrame.z));
		verts.push_back(glm::vec3(minFrame.x, minFrame.y, maxFrame.z));

		verts.push_back(glm::vec3(maxFrame.x, minFrame.y, minFrame.z));
		verts.push_back(glm::vec3(maxFrame.x, minFrame.y, maxFrame.z));

		verts.push_back(glm::vec3(minFrame.x, maxFrame.y, minFrame.z));
		verts.push_back(glm::vec3(minFrame.x, maxFrame.y, maxFrame.z));

		verts.push_back(glm::vec3(maxFrame.x, maxFrame.y, minFrame.z));
		verts.push_back(glm::vec3(maxFrame.x, maxFrame.y, maxFrame.z));

		verts.push_back(glm::vec3(maxFrame.x, maxFrame.y, maxFrame.z));
		verts.push_back(glm::vec3(maxFrame.x, minFrame.y, maxFrame.z));

		verts.push_back(glm::vec3(minFrame.x, maxFrame.y, maxFrame.z));
		verts.push_back(glm::vec3(minFrame.x, minFrame.y, maxFrame.z));

		verts.push_back(glm::vec3(minFrame.x, maxFrame.y, minFrame.z));
		verts.push_back(glm::vec3(minFrame.x, minFrame.y, minFrame.z));

		verts.push_back(glm::vec3(maxFrame.x, maxFrame.y, minFrame.z));
		verts.push_back(glm::vec3(maxFrame.x, minFrame.y, minFrame.z));

		verts.push_back(glm::vec3(minFrame.x, minFrame.y, minFrame.z));
		verts.push_back(glm::vec3(maxFrame.x, minFrame.y, minFrame.z));

		verts.push_back(glm::vec3(minFrame.x, maxFrame.y, minFrame.z));
		verts.push_back(glm::vec3(maxFrame.x, maxFrame.y, minFrame.z));

		verts.push_back(glm::vec3(minFrame.x, minFrame.y, maxFrame.z));
		verts.push_back(glm::vec3(maxFrame.x, minFrame.y, maxFrame.z));

		verts.push_back(glm::vec3(minFrame.x, maxFrame.y, maxFrame.z));
		verts.push_back(glm::vec3(maxFrame.x, maxFrame.y, maxFrame.z));

		int vertCount = verts.size();
		mPositionAttr->setData(verts);

		utility::ErrorState error;
		mMeshInstance->update(error);
	}

	bool FrameMesh::setup(nap::utility::ErrorState& error)
	{
		assert(mMeshInstance != nullptr);
		
		auto box = mFlexBlockMesh->getBox();
		auto frame = math::Box(mSize.x, mSize.y, mSize.z, glm::vec3(0,0,0));
		mBox = frame;

		// Create initial position data
		auto verts = std::vector<glm::vec3>();
		verts.clear();

		const glm::vec3& minBox = box.getMin();
		const glm::vec3& maxBox = box.getMax();
		const glm::vec3& minFrame = frame.getMin();
		const glm::vec3& maxFrame = frame.getMax();

		// control point 1
		verts.push_back(glm::vec3(minFrame.x, minFrame.y, maxFrame.z ));
		verts.push_back(glm::vec3(minBox.x, minBox.y, maxBox.z)); 

		// control point 2
		verts.push_back(glm::vec3(maxFrame.x, minFrame.y, maxFrame.z));
		verts.push_back(glm::vec3(maxBox.x, minBox.y, maxBox.z)); 

		// control point 3
		verts.push_back(glm::vec3(minFrame.x, maxFrame.y, maxFrame.z));
		verts.push_back(glm::vec3(minBox.x, maxBox.y, maxBox.z)); 

		// control point 4
		verts.push_back(glm::vec3(maxFrame.x, maxFrame.y, maxFrame.z));
		verts.push_back(glm::vec3(maxBox.x, maxBox.y, maxBox.z)); 

		// control point 5
		verts.push_back(glm::vec3(maxFrame.x, minFrame.y, minFrame.z));
		verts.push_back(glm::vec3(maxBox.x, minBox.y, minBox.z)); 

		// control point 6
		verts.push_back(glm::vec3(minFrame.x, minFrame.y, minFrame.z));
		verts.push_back(glm::vec3(minBox.x, minBox.y, minBox.z));  

		// control point 7
		verts.push_back(glm::vec3(maxFrame.x, maxFrame.y, minFrame.z));
		verts.push_back(glm::vec3(maxBox.x, maxBox.y, minBox.z)); 

		// control point 8
		verts.push_back(glm::vec3(minFrame.x, maxFrame.y, minFrame.z));
		verts.push_back(glm::vec3(minBox.x, maxBox.y, minBox.z));

		// construct frame
		verts.push_back(glm::vec3(minFrame.x, minFrame.y, minFrame.z));
		verts.push_back(glm::vec3(minFrame.x, minFrame.y, maxFrame.z));

		verts.push_back(glm::vec3(maxFrame.x, minFrame.y, minFrame.z));
		verts.push_back(glm::vec3(maxFrame.x, minFrame.y, maxFrame.z));

		verts.push_back(glm::vec3(minFrame.x, maxFrame.y, minFrame.z));
		verts.push_back(glm::vec3(minFrame.x, maxFrame.y, maxFrame.z));

		verts.push_back(glm::vec3(maxFrame.x, maxFrame.y, minFrame.z));
		verts.push_back(glm::vec3(maxFrame.x, maxFrame.y, maxFrame.z));

		verts.push_back(glm::vec3(maxFrame.x, maxFrame.y, maxFrame.z));
		verts.push_back(glm::vec3(maxFrame.x, minFrame.y, maxFrame.z));

		verts.push_back(glm::vec3(minFrame.x, maxFrame.y, maxFrame.z));
		verts.push_back(glm::vec3(minFrame.x, minFrame.y, maxFrame.z));

		verts.push_back(glm::vec3(minFrame.x, maxFrame.y, minFrame.z));
		verts.push_back(glm::vec3(minFrame.x, minFrame.y, minFrame.z));

		verts.push_back(glm::vec3(maxFrame.x, maxFrame.y, minFrame.z));
		verts.push_back(glm::vec3(maxFrame.x, minFrame.y, minFrame.z));

		verts.push_back(glm::vec3(minFrame.x, minFrame.y, minFrame.z));
		verts.push_back(glm::vec3(maxFrame.x, minFrame.y, minFrame.z));

		verts.push_back(glm::vec3(minFrame.x, maxFrame.y, minFrame.z));
		verts.push_back(glm::vec3(maxFrame.x, maxFrame.y, minFrame.z));

		verts.push_back(glm::vec3(minFrame.x, minFrame.y, maxFrame.z));
		verts.push_back(glm::vec3(maxFrame.x, minFrame.y, maxFrame.z));

		verts.push_back(glm::vec3(minFrame.x, maxFrame.y, maxFrame.z));
		verts.push_back(glm::vec3(maxFrame.x, maxFrame.y, maxFrame.z));

		int vertCount = verts.size();
		mMeshInstance->setNumVertices(vertCount);

		// Create position attribute
		mPositionAttr = &(mMeshInstance->getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getPositionName()));
		mPositionAttr->setData(verts);

		// Create initial color data
		std::vector<glm::vec4> colors(vertCount, { 1.0f, 0.0f, 0.0f, 1.0f });
		for (auto& color_attr : mColorAttrs)
			color_attr->setData(colors);

		// Set number of vertices
		mMeshInstance->setNumVertices(vertCount);

		// Draw as points
		MeshShape& shape = mMeshInstance->getShape(0);
		shape.setDrawMode(opengl::EDrawMode::LINES);

		// Automatically generate indices
		utility::generateIndices(shape, vertCount);

		return mMeshInstance->init(error);
	}
}