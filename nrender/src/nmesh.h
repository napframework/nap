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
	class Mesh;

	namespace vertex
	{
		/**
		* Helper object that binds a vertex attribute buy name to a vertex container
		*/
		class Attribute
		{
			friend class Mesh;
		public:
			Attribute(const std::string& name) : mID(name)	{ }
			virtual ~Attribute() = default;

			/**
			 *	@return the name of the vertex attribute
			 */
			const std::string& getName() const			{ return mID; }

			/**
			 * @return the vertex container that holds the GPU and CPU buffers
			 */
			VertexContainer* getContainer() const		{ return mContainer.get(); }

			// Move operator
			Attribute(Attribute&& other) : mID(other.mID), mContainer(std::move(other.mContainer))	{ }

		protected:
			// Associated vertex container, created when adding a vertex buffer to a mesh
			std::unique_ptr<VertexContainer> mContainer = nullptr;

			// Name of the attribute
			std::string mID;
		};
	}

	class Mesh
	{
	public:
		using VertexAttributeID = std::string;

		/**
		* Known vertex attribute IDs in the system, used for loading/creating meshes with well-known attributes.
		*/
		struct VertexAttributeIDs
		{
			static const VertexAttributeID PositionVertexAttr;	//< Default position vertex attribute name
			static const VertexAttributeID NormalVertexAttr;		//< Default normal vertex attribute name
			static const VertexAttributeID UVVertexAttr;			//< Default uv vertex attribute name
			static const VertexAttributeID ColorVertexAttr;		//< Default color vertex attribute name

			/**
			 * Returns the name of the vertex uv attribute based on the queried uv channel
			 * @param uvChannel: the uv channel index to query
			 * @return the name of the vertex attribute
			 */
			static const VertexAttributeID GetUVVertexAttr(int uvChannel);

			/**
			 *	Returns the name of the vertex color attribute based on the queried uv channel
			 * @param colorChannel: the color channel index to query
			 * @return the name of the color vertex attribute
			 */
			static const VertexAttributeID GetColorVertexAttr(int colorChannel);
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
		template<typename T>
		void addVertexAttribute(const VertexAttributeID& id, const T* data);

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
		const IndexBuffer* getIndexBuffer() const;

		/**
		* @return the number of vertices associated with the data in the vertex buffers.
		*/
		unsigned int getVertCount() const	{ return mNumVertices; }

		/**
		* @return the drawmode as set in the constructor.
		*/
		EDrawMode getDrawMode() const		{ return mDrawMode;  }

	private:
		int										mNumVertices = 0;					//< Total number of vertices
		std::vector<vertex::Attribute>		mAttributes;						//< All vertex buffers associated with this mesh
		std::unique_ptr<IndexContainer>			mIndices = nullptr;					//< Mesh connectivity
		EDrawMode								mDrawMode = EDrawMode::TRIANGLES;	//< Draw mode
	};


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	void opengl::Mesh::addVertexAttribute(const VertexAttributeID& id, const T* data)
	{
		// Create the attribute
		opengl::vertex::Attribute attribute(id);
		 
		// Make the buffer and copy data
		std::unique_ptr<TypedVertexContainer<T>> new_container = std::make_unique<TypedVertexContainer<T>>();
		bool success = new_container->copyData(mNumVertices, data);
		assert(success);

		// Upload data to GPU
		new_container->sync();

		// Set the container
		attribute.mContainer = std::move(new_container);

		// Add the attribute
		mAttributes.emplace_back(std::move(attribute));
	}
} // opengl
