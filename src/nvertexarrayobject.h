#pragma once

// Local Includes
#include "nvertexbuffer.h"
#include "nindexbuffer.h"
#include "ndrawutils.h"

// External Includes
#include <GL/glew.h>
#include <unordered_map>

namespace opengl
{
	/**
	 * Organizes and stores a set of Vertex Data objects that can be rendered
	 *
	 * The VertexArrayObject is wraps an OpenGL Vertex Array Object
	 * It allows for setting up the necessary objects for rendering with
	 * a particular shader program
	 *
	 * The VertexArrayObject works internally with vertex buffers
	 * Vertex buffers are the actual data containers
	 * The VertexArrayObject organizes the data containers in order to render them in one pass
	 * 
	 * This object does not own the buffers it organizes!
	 * For more information: https://www.opengl.org/wiki/Tutorial2:_VAOs,_VBOs,_Vertex_and_Fragment_Shaders_(C_/_SDL)
	 */
	class VertexArrayObject
	{
	public:
		// Construction / Destruction
		VertexArrayObject();
		virtual ~VertexArrayObject();

		// Copy is not allowed
		VertexArrayObject(const VertexArrayObject& other) = delete;
		VertexArrayObject& operator=(const VertexArrayObject& other) = delete;

		/**
		 * Draws the vertex data associated with this buffer object to the currently active context
		 * Calls bind before drawing
		 *
		 * @param mode:  The vertex connection mode (GL_TRIANGLES, GL_LINE_STRIP etc)
		 * @param count: Number of vertices to draw, -1 draws all associated vertices
		 */
		void			draw(int count = -1);

		/**
		 * Specify which index buffer to use with this vertex array object
		 * Not specifying one will leave indexing out of the programmable pipe
		 * @param buffer: the index buffer that describes attribute connectivity
		 */
		bool			setIndexBuffer(IndexBuffer& buffer);

		/**
		 * Adds a vertex buffer to this vertex array object
		 * Note that this call will find a currently available index and use that one
		 * If calling multiple times in a row, the assigned id will be increment by one each time
		 * @ return the assigned vertex attribute index, -1 if unsuccessful
		 */
		unsigned int	addVertexBuffer(VertexBuffer& buffer);

		/**
		 * Adds and binds a vertex buffer to this vertex array object
		 * Note that this call will bind the buffer at the designated index
		 * @ return the assigned vertex attribute index, -1 if unsuccessful
		 */
		bool			addVertexBuffer(unsigned int index, VertexBuffer& buffer);

		/**
		 * @return the vertex buffer binding index associated with this VAO
		 * On failure to retrieve the index the output will always be -1
		 * This index is important when enabling the VAO for drawing with a shader
		 * The index can be bound to a vertex shader attribute location using a name
		 */
		int				getVertexBufferIndex(const VertexBuffer& buffer) const;

		/**
		* @return the number of vertices associated with the data in the vertex buffers
		* Note that for every added buffer the vertex count is checked against currently available vertex count
		* If the vertex count differs from buffer to buffer the lowest common denominator is picked
		*/
		unsigned int	getVertCount() const			{ return mVertCount; }

		/**
		 * Sets the mode used when drawing this object to a render target
		 * @param mode the new draw mode
		 */
		void			setDrawMode(DrawMode mode);

		/**
		 * Returns the current draw mode
		 * @param the current draw mode
		 */
		DrawMode		getDrawMode() const				{ return mDrawMode; }
	
	private:
		/**
		 * Creates the vertex buffer for the specified GL context
		 *
		 * This needs be called after creation to ensure the associated data can be rendered
		 */
		void			allocate(GLContext glContext);
		
		/**
		 * Destroys the vertex buffer for the specified GL context
		 *
		 * This needs be called after creation to ensure the associated data can be rendered
		 */
		void			deallocate(GLContext glContext);

		/**
		 * Binds the Vertex Buffer to be used by subsequent vertex buffer calls for the specified GL context
		 */
		bool			bind(GLContext glContext);

		/**
		 * Unbinds the Vertex Buffer for the specified GL context
		 */
		bool			unbind(GLContext glContext);

		/**
		 * Returns if the buffer is generated and can be used for rendering by the specified GL context
		 */
		bool			isAllocated(GLContext glContext) const;

		/**
		 * Recreate the vertex array object and and its associated resources for the specified context
		 * Note: it is assumed that the specified context is also the *currently active* context
		 */
		bool			recreate(GLContext context);

	private:
		/**
		 * VertexArrayState represents the state of a VAO for a specific OpenGL Context.
		 * It is used to be able to share a single CPU VAO over multiple contexts
		 */
		struct VertexArrayState
		{
			GLuint 	mId = 0;			// The ID of the VAO
			bool	mIsDirty = false;	// Whether the VAO is dirty
		};

		using VertexBufferBindings = std::unordered_map<unsigned int, VertexBuffer*>;
		using ContextSpecificStateMap = std::unordered_map<GLContext, VertexArrayState>;
		ContextSpecificStateMap	mContextSpecificState;				// The per-context vertex array state
		DrawMode				mDrawMode = DrawMode::TRIANGLES;	// Mode currently used for drawing

		/**
		 * Maps a buffer to a specific pointer index in the vertex array object
		 * This index is important when enabling the VAO for drawing with a shader
		 * The index can be bound to a vertex shader attribute location using a name
		 * For that reason it's important to associate an id with every added buffer
		 */
		VertexBufferBindings	mBindings;				
		unsigned int			mVertCount = 0;						// Total number of vertices

		/**
		 * Defines the currently used index buffer that defines vertex connectivity
		 * Not having an index buffer associated with this vertex array object will
		 * use the default array draw method
		 */
		IndexBuffer*			mIndexBuffer = nullptr;				
	};
}
