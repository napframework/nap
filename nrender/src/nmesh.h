#pragma once

// Local Includes
#include "nvertexcontainer.h"
#include "nvertexarrayobject.h"
#include "nglutils.h"
#include "nindexcontainer.h"

// External Includes
#include <memory>
#include <vector>
#include <sstream>

namespace opengl
{
	/**
	 * Defines a polygonal mesh
	 * Every mesh has a number of vertex attributes that are identified through an ID.
	 */
	class Mesh
	{
	public:
		using VertexAttributeID = std::string;

		/**
		* Known vertex attribute IDs in the system, used for loading/creating meshes with well-known attributes.
		*/
		struct VertexAttributeIDs
		{
			static const VertexAttributeID PositionVertexAttr;
			static const VertexAttributeID NormalVertexAttr;
			static const VertexAttributeID UVVertexAttr;
			static const VertexAttributeID ColorVertexAttr;

			static const VertexAttributeID GetUVVertexAttr(int uvChannel)
			{
				std::ostringstream stream;
				stream << UVVertexAttr << uvChannel;
				return stream.str();
			}

			static const VertexAttributeID GetColorVertexAttr(int colorChannel)
			{
				std::ostringstream stream;
				stream << ColorVertexAttr << colorChannel;
				return stream.str();
			}
		};


		// Default Constructor
		Mesh(int numVertices, EDrawMode drawMode);

		// Default destructor
		virtual ~Mesh() = default;

		// Copy is not allowed
		// TODO: Add copy method
		Mesh(const Mesh& other) = delete;
		Mesh& operator=(const Mesh& other) = delete;

		/**
		* Adds a vertex attribute stream to the mesh
		* @param id: name of the vertex attribute
		* @param components: number of component per element (for instance, 3 for vector with 3 floats)
		* @param data: Pointer to array containing attribute data.
		*/
		void addVertexAttribute(const VertexAttributeID& id, unsigned int components, const float* data);

		/**
		* @return Returns pointer to the attribute buffer if found, otherwise nullptr.
		* @param id: name of the vertex attribute
		*/
		const VertexAttributeBuffer* findVertexAttributeBuffer(const VertexAttributeID& id) const;

		/**
		 * Adds a set of indices to the mesh. Without indices the regular
		 * array draw method applies, with indices the mesh will be drawn
		 * using the connectivity described by the indices
		 * @param count: The number of indices that will be copied
		 * @param indices: The array of indices that will describe mesh connectivity
		 */
		void setIndices(unsigned int count, const unsigned int* data);

		/**
		* @return The indexbuffer if set, otherwise nullptr.
		*/
		const IndexBuffer* getIndexBuffer() const 
		{ 
			return mIndices->getIndexBuffer();  
		}

		/**
		* @return the number of vertices associated with the data in the vertex buffers.
		*/
		unsigned int getVertCount() const { return mNumVertices; }

		/**
		* @return the drawmode as set in the constructor.
		*/
		EDrawMode getDrawMode() const { return mDrawMode;  }

	private:
		struct Attribute
		{
			std::string mID;
			std::unique_ptr<FloatVertexContainer> mData;

			Attribute() = default;
			Attribute(Attribute&& other) : mID(other.mID), mData(std::move(other.mData)) {}
		};

		int										mNumVertices = 0;
		std::vector<Attribute>					mAttributes;
		std::unique_ptr<IndexContainer>			mIndices = nullptr;					//< Mesh connectivity
		EDrawMode								mDrawMode = EDrawMode::TRIANGLES;	//< Draw mode
	};

} // opengl
