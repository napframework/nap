#include "meshutils.h"
#include <mathutils.h>
#include <glm/gtx/normal.hpp>

namespace nap
{
	class TriangleIterator
	{
	public:
		TriangleIterator(const SubMesh& subMesh, int startIndex) :
			mSubMesh(&subMesh),
			mCurrentIndex(startIndex),
			mNumIndices(subMesh.getNumIndices()) 
		{
		}

		bool isDone() const
		{
			return mCurrentIndex >= mNumIndices;
		}

		virtual const glm::ivec3 next() = 0;

	protected:
		const SubMesh*	mSubMesh;
		int				mCurrentIndex;
		int				mNumIndices;
	};

	class TriangleListIterator : public TriangleIterator
	{
	public:
		TriangleListIterator(const SubMesh& subMesh) :
			TriangleIterator(subMesh, 0)
		{
			assert(subMesh.getDrawMode() == opengl::EDrawMode::TRIANGLES);
			assert(mNumIndices != 0 && mNumIndices % 3 == 0);
		}

		virtual const glm::ivec3 next() override
		{
			glm::ivec3 result;

			const unsigned int* id = mSubMesh->getIndices().data() + mCurrentIndex;
			result.x = *(id + 0);
			result.y = *(id + 1);
			result.z = *(id + 2);

			mCurrentIndex += 3;

			return result;
		}
	};

	class TriangleFanIterator : public TriangleIterator
	{
	public:
		TriangleFanIterator(const SubMesh& subMesh) :
			TriangleIterator(subMesh, 2)
		{
			assert(subMesh.getDrawMode() == opengl::EDrawMode::TRIANGLE_FAN);
			assert(mNumIndices >= 3);
		}

		virtual const glm::ivec3 next() override
		{
			glm::ivec3 result;
			const unsigned int* id = mSubMesh->getIndices().data();
			result.x = *id;
			result.y = *(id + mCurrentIndex - 1);
			result.z = *(id + mCurrentIndex);
			++mCurrentIndex;

			return result;
		}
	};

	class TriangleStripIterator : public TriangleIterator
	{
	public:
		TriangleStripIterator(const SubMesh& subMesh) :
			TriangleIterator(subMesh, 2)
		{
			assert(subMesh.getDrawMode() == opengl::EDrawMode::TRIANGLE_STRIP);
			assert(mNumIndices >= 3);
		}

		virtual const glm::ivec3 next() override
		{
			glm::ivec3 result;
			const unsigned int* id = mSubMesh->getIndices().data() + (mCurrentIndex - 2);
			result.x = *(id + 0);
			result.y = *(id + 1);
			result.z = *(id + 2);
			++mCurrentIndex;

			return result;
		}
	};


	class TriangleSubMeshIterator
	{
	public:
		TriangleSubMeshIterator(const MeshInstance& meshInstance) :
			mMeshInstance(&meshInstance),
			mCurSubMesh(0),
			mCurrentTriangleIndex(0)
		{
			createIteratorForNextSubMesh();
		}

		bool isDone() const
		{
			return mCurIterator == nullptr;
		}

		const glm::ivec3 next()
		{
			glm::ivec3 result = mCurIterator->next();
			if (mCurIterator->isDone())
				createIteratorForNextSubMesh();

			++mCurrentTriangleIndex;
			return result;
		}

		int getCurrentSubMeshIndex() const { return mCurSubMesh; }
		int getCurrentTriangleIndex() const { return mCurrentTriangleIndex; }
	
	private:
		void createIteratorForNextSubMesh()
		{
			mCurIterator.reset();
			for (; mCurSubMesh < mMeshInstance->getNumSubMeshes() && mCurIterator == nullptr; ++mCurSubMesh)
			{
				const SubMesh& subMesh = mMeshInstance->getSubMesh(mCurSubMesh);

				switch (subMesh.getDrawMode())
				{
				case opengl::EDrawMode::TRIANGLES:
					mCurIterator = std::make_unique<TriangleListIterator>(subMesh);
					break;
				case opengl::EDrawMode::TRIANGLE_STRIP:
					mCurIterator = std::make_unique<TriangleStripIterator>(subMesh);
					break;
				case opengl::EDrawMode::TRIANGLE_FAN:
					mCurIterator = std::make_unique<TriangleFanIterator>(subMesh);
					break;
				default:
					break;
				}
			}
		}

	private:
		const MeshInstance* mMeshInstance;
		std::unique_ptr<TriangleIterator> mCurIterator;
		int mCurSubMesh;
		int mCurrentTriangleIndex;
	};


	bool NAPAPI isTriangleMesh(const SubMesh& subMesh)
	{
		switch (subMesh.getDrawMode())
		{
		case opengl::EDrawMode::LINE_LOOP:
		case opengl::EDrawMode::LINE_STRIP:
		case opengl::EDrawMode::LINES:
		case opengl::EDrawMode::POINTS:
		case opengl::EDrawMode::UNKNOWN:
			return false;
		case opengl::EDrawMode::TRIANGLES:
		case opengl::EDrawMode::TRIANGLE_FAN:
		case opengl::EDrawMode::TRIANGLE_STRIP:
			return true;
		default:
			assert(false);
			return false;
		}
	}
	
	void NAPAPI setTriangleIndices(SubMesh& mesh, int number, glm::ivec3& indices)
	{		
		// Copy triangle index over
		SubMesh::IndexList& mesh_indices = mesh.getIndices();

		switch (mesh.getDrawMode())
		{
		case opengl::EDrawMode::TRIANGLES:
		{
			// Make sure our index is in range
			assert((number * 3) + 2 < mesh_indices.size());

			// Fill the data
			unsigned int* id = mesh_indices.data() + (number * 3);
			*(id + 0) = indices.x;
			*(id + 1) = indices.y;
			*(id + 2) = indices.z;
			break;
		}
		case opengl::EDrawMode::TRIANGLE_FAN:
		{
			assert(number + 2 < mesh_indices.size());
			unsigned int* id = mesh_indices.data();
			*id = indices.x;
			*(id + number + 1) = indices.y;
			*(id + number + 2) = indices.z;
			break;
		}
		case opengl::EDrawMode::TRIANGLE_STRIP:
		{
			assert(number + 2 < mesh_indices.size());
			unsigned int* id = mesh_indices.data() + number;
			*(id + 0) = indices.x;
			*(id + 1) = indices.y;
			*(id + 2) = indices.z;
			break;
		}
		default:
			assert(false);
			break;
		}
	}


	void computeBoundingBox(const MeshInstance& mesh, math::Box& outBox)
	{
		glm::vec3 min = { nap::math::max<float>(), nap::math::max<float>(), nap::math::max<float>() };
		glm::vec3 max = { nap::math::min<float>(), nap::math::min<float>(), nap::math::min<float>() };

		const nap::VertexAttribute<glm::vec3>& positions = mesh.getAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
		for (const auto& point : positions.getData())
		{
			if (point.x < min.x) { min.x = point.x; }
			if (point.x > max.x) { max.x = point.x; }
			if (point.y < min.y) { min.y = point.y; }
			if (point.y > max.y) { max.y = point.y; }
			if (point.z < min.z) { min.z = point.z; }
			if (point.z > max.z) { max.z = point.z; }
		}
		outBox.mMinCoordinates = min;
		outBox.mMaxCoordinates = max;
	}


	nap::math::Box computeBoundingBox(const MeshInstance& mesh)
	{
		math::Box box;
		computeBoundingBox(mesh, box);
		return box;
	}


	void computeNormals(const MeshInstance& meshInstance, const VertexAttribute<glm::vec3>& positions, VertexAttribute<glm::vec3>& outNormals)
	{
		assert(outNormals.getCount() == positions.getCount());
		
		// Total number of attributes
		int attr_count = positions.getCount();

		// Normal data
		const std::vector<glm::vec3>& position_data = positions.getData();
		std::vector<glm::vec3>& normal_data = outNormals.getData();
		
		// Reset normal data so we can accumulate data into it
		for (glm::vec3& normal : normal_data)
			normal = glm::vec3(0.0f, 0.0f, 0.0f);

		// Accumulate all normals into the normals array
		TriangleSubMeshIterator iterator(meshInstance);
		while (!iterator.isDone())
		{
			glm::ivec3 indices = iterator.next();

			glm::vec3 point0 = position_data[indices[0]];
			glm::vec3 point1 = position_data[indices[1]];
			glm::vec3 point2 = position_data[indices[2]];

			glm::vec3 normal = glm::cross((point0 - point1), (point0 - point2));
			normal_data[indices[0]] += normal;
			normal_data[indices[1]] += normal;
			normal_data[indices[2]] += normal;
		}

		// Normalize to deal with shared vertices
		for (glm::vec3& normal : normal_data)
			normal = glm::normalize(normal);
	}


	void NAPAPI reverseWindingOrder(MeshInstance& mesh)
	{
		TriangleSubMeshIterator iterator(mesh);
		while (!iterator.isDone())
		{
			glm::ivec3 cindices = iterator.next();
			std::swap(cindices.x, cindices.z);

			int subMeshIndex = iterator.getCurrentSubMeshIndex();
			int triangleIndex = iterator.getCurrentTriangleIndex();
			SubMesh& curSubMesh = mesh.getSubMesh(subMeshIndex);

			setTriangleIndices(curSubMesh, triangleIndex, cindices);
		}
	}

	void NAPAPI generateIndices(nap::SubMesh& subMesh, int vertexCount)
	{
		SubMesh::IndexList& indices = subMesh.getIndices();
		indices.resize(vertexCount);
		for (int vertex = 0; vertex < vertexCount; ++vertex)
			indices[vertex] = vertex;
	}
}