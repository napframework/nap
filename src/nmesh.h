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
		Mesh();

		// Default destructor
		virtual ~Mesh() = default;

		// Copy is not allowed
		// TODO: Add copy method
		Mesh(const Mesh& other) = delete;
		Mesh& operator=(const Mesh& other) = delete;

		/**
		 * Adds the vertices
		 * Needs to consist out of 3 float components (x, y, z)
		 * Performs an actual copy operation
		 * Already allocated vertex data is cleared
		 * @param vertices total number of vertices held by data
		 * @param data vertex data to copy
		 */
		void copyVertexData(unsigned int vertices, float* data);

		/**
		 * Adds the normals
		 * Needs to consist out of 3 float components (x, y, z)
		 * Performs an actual copy operation
		 * Already allocated normal data is cleared
		 * @param vertices total number of vertices held by data
		 * @param data normal data to copy
		 */
		void copyNormalData(unsigned int vertices, float* data);

		/**
		 * Adds a set of Colors at the next available slot
		 * Can consist out of 3 or 4 components, ie: rgb or rgba
		 * Performs an actual copy operation
		 */
		void copyColorData(unsigned int components, unsigned int vertices, float* data);

		/**
		 * Adds a set of UVs at the next available slot
		 * Can consist out of 2 or 3 components, ie: xy, or xyz
		 * Performs an actual copy operation
		 */
		void copyUVData(unsigned int components, unsigned int vertices, float* data);

		/**
		 * Adds a set of indices to the mesh. Indices are created
		 * if they haven't been created yet. Without indices the regular
		 * array draw method applies, with indices the mesh will be drawn
		 * using the connectivity described by the indices
		 * @param count: The number of indices that will be copied
		 * @param indices: The array of indices that will describe mesh connectivity
		 */
		void copyIndexData(unsigned int count, unsigned int* data);

		/**
		* Draws the vertex data associated with this mesh object to the currently active context
		* Calls bind before drawing.
		*/
		virtual void draw();

		/**
		* @return the number of vertices associated with the data in the vertex buffers
		* Note that for every set the vertex count is checked against currently available vertex count
		* If the vertex count differs from buffer to buffer the lowest common denominator is picked
		*/
		unsigned int getVertCount() const { return mObject.getVertCount(); }

		/**
		 * @return the number of color channels
		 */
		unsigned int getColorChannels() const	{ return static_cast<unsigned int>(mColors.size()); }

		/**
		 * @return the number of uv channels
		 */
		unsigned int getUvChannels() const		{ return static_cast<unsigned int>(mUvs.size()); }

		/**
		 * @return if the mesh has uv's
		 */
		bool hasUvs() const						{ return mUvs.size() != 0; }

		/**
		 * @return if the mesh has normals
		 */
		bool hasNormals() const					{ return mNormals != nullptr; }

		/**
		 * @return if the mesh has vertices
		 */
		bool hasVertices() const				{ return mVertices != nullptr; }

		/**
		 * @return if the mesh has colors
		 */
		bool hasColors() const					{ return mColors.size() != 0; }

		/**
		 * @reutrn if the mesh uses indices
		 */
		bool hasIndices() const					{ return mIndices != nullptr; }

		/**
		 * @return the vertex buffer binding index associated with the vertex buffer, -1 if not found
		 * The index can be bound to a vertex shader attribute location using a name
		 */
		int getVertexBufferIndex() const;

		/**
		 * @return the normal buffer binding index, -1 if not found
		 * The index can be bound to a vertex shader attribute location using a name
		 */
		int getNormalBufferIndex() const;

		/**
		 * @return the color buffer binding index, -1 if not found
		 * The index can be bound to a vertex shader attribute location using a name
		 * @param colorChannel the color channel to query the vertex binding index for
		 */
		int getColorBufferIndex(unsigned int colorChannel = 0) const;

		/**
		 * @return the uv buffer binding index, -1 if not found
		 * The index can be bound to a vertex shader attribute location using a name
		 * @param uvChannel the UV channel to query the vertex binding index for
		 */
		int getUvBufferIndex(unsigned int uvChannel = 0) const;

		/**
		* Sets the mode used when drawing this object to a render target
		* @param mode the new draw mode
		*/
		void setDrawMode(DrawMode mode)								{ mObject.setDrawMode(mode); }

		/**
		* Returns the current draw mode
		* @param the current draw mode
		*/
		DrawMode getDrawMode() const								{ return mObject.getDrawMode(); }

	protected:
		VertexArrayObject						mObject;				// Contains all the vertex buffers

	private:
		using VertexContainerArray = std::vector<std::unique_ptr<FloatVertexContainer>>;

		std::unique_ptr<FloatVertexContainer>	mVertices = nullptr;	//< Mesh Positions
		std::unique_ptr<FloatVertexContainer>	mNormals  = nullptr;	//< Mesh Normals
		VertexContainerArray					mUvs;					//< Mesh Uv's
		VertexContainerArray					mColors;				//< Mesh Color's
		std::unique_ptr<IndexContainer>			mIndices = nullptr;		//< Mesh Indices

		/**
		 * Creates or updates a vertex container associated with this mesh
		 * Utility function that handles most of the containers mentioned above (vertices, color etc.)
		 * Giving location a nullptr will create a new buffer
		 */
		template <typename T>
		void updateVertexContainer(std::unique_ptr<TypedVertexContainer<T>>& location, unsigned int components, unsigned int verts, T* data);

		/**
		 * Creates or updates the index container associated with this mesh
		 */
		void updateIndexContainer(std::unique_ptr<IndexContainer>& location, unsigned int count, unsigned int* data);

		/**
		 * Utility that is used for retrieving the binding for @container
		 */
		int getContainerBindingIndex(VertexContainer* container) const;
	};

} // opengl

#include "nmesh.hpp"
