#pragma once

// Local Includes
#include "nvertexcontainer.h"
#include "nvertexarrayobject.h"
#include "nglutils.h"
#include "nindexcontainer.h"

// External Includes
#include <memory>
#include <vector>

namespace opengl
{
	using VertexAttributeID = std::string;
	struct VertexAttributeIDs
	{
		static const VertexAttributeID PositionVertexAttr;
		static const VertexAttributeID NormalVertexAttr;
		static const VertexAttributeID UVVertexAttr;
		static const VertexAttributeID ColorVertexAttr;
	};

	/**
	 * Defines a polygonal mesh
	 * Every mesh contains multiple sets of vertices in the form of VertexContainers
	 * The vertex containers are grouped and drawn using a vertex array object
	 * The mesh owns all of it's vertex data, both on the CPU and GPU side
	 * The binding order of the vertex buffers depends on the order data is added
	 * Every mesh consists out of a default set of VertexContainers, including:
	 *
	 * Position
	 * Normals
	 * UV coordinates	
	 * Color
	 * 
	 * Every mesh can have multiple UV and Color sets 
	 * and only 1 position and normal set
	 * When adding data make sure the vertex count is the same for every set of vertices
	 * This object will happily allocate and create the buffers for you but it does
	 * not perform any bound checks! It's up to the user to make sure vertex count is similar for every set!
	 */
	class Mesh
	{
	public:
		// Default Constructor
		Mesh(int numVertices);

		// Default destructor
		virtual ~Mesh() = default;

		// Copy is not allowed
		// TODO: Add copy method
		Mesh(const Mesh& other) = delete;
		Mesh& operator=(const Mesh& other) = delete;

		void addVertexAttribute(const VertexAttributeID& id, unsigned int components, const float* data);
		const VertexBuffer* findVertexAttributeBuffer(const VertexAttributeID& id) const;

		/**
		 * Adds a set of indices to the mesh. Indices are created
		 * if they haven't been created yet. Without indices the regular
		 * array draw method applies, with indices the mesh will be drawn
		 * using the connectivity described by the indices
		 * @param count: The number of indices that will be copied
		 * @param indices: The array of indices that will describe mesh connectivity
		 */
		void setIndices(unsigned int count, const unsigned int* data);

		const IndexBuffer* getIndexBuffer() const { return mIndices->getIndexBuffer();  }

		/**
		* @return the number of vertices associated with the data in the vertex buffers
		* Note that for every set the vertex count is checked against currently available vertex count
		* If the vertex count differs from buffer to buffer the lowest common denominator is picked
		*/
		unsigned int getVertCount() const { return mNumVertices; }

	private:
		using VertexContainerArray = std::vector<std::unique_ptr<FloatVertexContainer>>;

		struct Attribute
		{
			std::string mID;
			std::unique_ptr<FloatVertexContainer> mData;

			Attribute() = default;
			Attribute(Attribute&& other) : mID(other.mID), mData(std::move(other.mData)) {}
		};

		int										mNumVertices = 0;
		std::vector<Attribute>					mAttributes;
		std::unique_ptr<IndexContainer>			mIndices = nullptr;		//< Mesh Indices
	};

} // opengl
