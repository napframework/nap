#include "meshutils.h"
#include <mathutils.h>
#include <glm/gtx/normal.hpp>

namespace nap
{
	class TriangleIterator
	{
	public:
		TriangleIterator(const MeshShape& shape, int startIndex) :
			mShape(&shape),
			mCurrentIndex(shape.getIndices().data()),
			mIndexEnd(shape.getIndices().data() + shape.getIndices().size()) 
		{
		}

		bool isDone() const
		{
			return mCurrentIndex >= mIndexEnd;
		}

		virtual const glm::ivec3 next() = 0;

	protected:
		const MeshShape*	mShape;
		const unsigned int*	mCurrentIndex;
		const unsigned int*	mIndexEnd;
	};

	class TriangleListIterator final : public TriangleIterator
	{
	public:
		TriangleListIterator(const MeshShape& shape) :
			TriangleIterator(shape, 0)
		{
			assert(shape.getDrawMode() == opengl::EDrawMode::TRIANGLES);
			assert(shape.getNumIndices() != 0 && shape.getNumIndices() % 3 == 0);
		}

		virtual const glm::ivec3 next() override
		{
			const unsigned int* currentIndex = mCurrentIndex;

			glm::ivec3 result;		
			result.x = *currentIndex++;
			result.y = *currentIndex++;
			result.z = *currentIndex++;

			mCurrentIndex = currentIndex;

			return result;
		}
	};

	class TriangleFanIterator final : public TriangleIterator
	{
	public:
		TriangleFanIterator(const MeshShape& shape) :
			TriangleIterator(shape, 2)
		{
			assert(shape.getDrawMode() == opengl::EDrawMode::TRIANGLE_FAN);
			assert(shape.getNumIndices() >= 3);
			
			mFanStartIndex = shape.getIndices().front();
		}

		virtual const glm::ivec3 next() override
		{
			const unsigned int* currentIndex = mCurrentIndex;

			glm::ivec3 result;
			result.x = mFanStartIndex;
			result.y = *(currentIndex - 1);
			result.z = *currentIndex;
			
			++mCurrentIndex;

			return result;
		}

	private:
		unsigned int mFanStartIndex;
	};

	class TriangleStripIterator final : public TriangleIterator
	{
	public:
		TriangleStripIterator(const MeshShape& shape) :
			TriangleIterator(shape, 2)
		{
			assert(shape.getDrawMode() == opengl::EDrawMode::TRIANGLE_STRIP);
			assert(shape.getNumIndices() >= 3);
		}

		virtual const glm::ivec3 next() override
		{
			const unsigned int* currentIndex = mCurrentIndex-2;

			glm::ivec3 result;
			result.x = *(currentIndex + 0);
			result.y = *(currentIndex + 1);
			result.z = *(currentIndex + 2);

			++mCurrentIndex;

			return result;
		}
	};


	class TriangleShapeIterator final
	{
	public:
		TriangleShapeIterator(const MeshInstance& meshInstance) :
			mMeshInstance(&meshInstance),
			mCurIterator(nullptr),
			mCurShapeIndex(0),
			mCurrentTriangleIndex(0)
		{
			createIteratorForNextShape();
		}

		~TriangleShapeIterator()
		{
			delete mCurIterator;
		}

		bool isDone() const
		{
			return mCurIterator == nullptr;
		}

		const glm::ivec3 next()
		{
			glm::ivec3 result = mCurIterator->next();
			if (mCurIterator->isDone())
				createIteratorForNextShape();

			++mCurrentTriangleIndex;
			return result;
		}

		int getCurrentShapeIndex() const { return mCurShapeIndex; }
		int getCurrentTriangleIndex() const { return mCurrentTriangleIndex; }
	
	private:
		void createIteratorForNextShape()
		{
			delete mCurIterator;
			mCurIterator = nullptr;

			for (; mCurShapeIndex < mMeshInstance->getNumShapes() && mCurIterator == nullptr; ++mCurShapeIndex)
			{
				const MeshShape& shape = mMeshInstance->getShape(mCurShapeIndex);

				switch (shape.getDrawMode())
				{
				case opengl::EDrawMode::TRIANGLES:
					mCurIterator = new TriangleListIterator(shape);
					break;
				case opengl::EDrawMode::TRIANGLE_STRIP:
					mCurIterator = new TriangleStripIterator(shape);
					break;
				case opengl::EDrawMode::TRIANGLE_FAN:
					mCurIterator = new TriangleFanIterator(shape);
					break;
				default:
					break;
				}
			}
		}

	private:
		const MeshInstance* mMeshInstance;
		TriangleIterator* mCurIterator;		// Note: not using unique_ptr to avoid overhead when dereffing pointer during iteration
		int mCurShapeIndex;
		int mCurrentTriangleIndex;
	};


	bool NAPAPI isTriangleMesh(const MeshShape& shape)
	{
		switch (shape.getDrawMode())
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
	
	void NAPAPI setTriangleIndices(MeshShape& mesh, int number, glm::ivec3& indices)
	{		
		// Copy triangle index over
		MeshShape::IndexList& mesh_indices = mesh.getIndices();

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
		std::memset(normal_data.data(), 0, sizeof(glm::vec3) * normal_data.size());

		// Accumulate all normals into the normals array
		const glm::vec3* position_data_ptr = position_data.data();
		glm::vec3* normal_data_ptr = normal_data.data();

		TriangleShapeIterator iterator(meshInstance);
		while (!iterator.isDone())
		{
			glm::ivec3 indices = iterator.next();

			const glm::vec3& point0 = position_data_ptr[indices[0]];
			const glm::vec3& point1 = position_data_ptr[indices[1]];
			const glm::vec3& point2 = position_data_ptr[indices[2]];

			glm::vec3 normal = glm::cross((point0 - point1), (point0 - point2));

			normal_data_ptr[indices[0]] += normal;
			normal_data_ptr[indices[1]] += normal;
			normal_data_ptr[indices[2]] += normal;
		}

		// Normalize to deal with shared vertices
		for (glm::vec3& normal : normal_data)
			normal = glm::normalize(normal);
	}


	void NAPAPI reverseWindingOrder(MeshInstance& mesh)
	{
		TriangleShapeIterator iterator(mesh);
		while (!iterator.isDone())
		{
			glm::ivec3 cindices = iterator.next();
			std::swap(cindices.x, cindices.z);

			int shape_index = iterator.getCurrentShapeIndex();
			int triangle_index = iterator.getCurrentTriangleIndex();
			MeshShape& shape = mesh.getShape(shape_index);

			setTriangleIndices(shape, triangle_index, cindices);
		}
	}

	void NAPAPI generateIndices(nap::MeshShape& shape, int vertexCount)
	{
		MeshShape::IndexList& indices = shape.getIndices();
		indices.resize(vertexCount);
		for (int vertex = 0; vertex < vertexCount; ++vertex)
			indices[vertex] = vertex;
	}
}