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
	 * Interface used for OpenGL resources that are context dependent. Because they are context dependent,
	 * the correct OpenGL context must be set when destroying these objects, otherwise context from other
	 * contexts may accidentally be destroyed. 
	 * To avoid numerous context switches for each destroy, IGLContextResource is used to perform batch destruction
	 * of such resources per context.
	 */
	class IGLContextResource
	{
	public:
		/**
		 * Used for destruction of a resource that is context dependent.
		 * @param context: the context to destroy the resource for. This is also the active GL context.
		 */
		virtual void destroy(opengl::GLContext context) = 0;
	};


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
	class VertexArrayObject : public IGLContextResource
	{
	public:

		VertexArrayObject() = default;
		~VertexArrayObject();

		virtual void destroy(opengl::GLContext context) override;

		// Copy is not allowed
		VertexArrayObject(const VertexArrayObject& other) = delete;
		VertexArrayObject& operator=(const VertexArrayObject& other) = delete;

		/**
		 * Adds and binds a vertex buffer to this vertex array object
		 * Note that this call will bind the buffer at the designated index
		 * @ return the assigned vertex attribute index, -1 if unsuccessful
		 */
		void addVertexBuffer(unsigned int index, const VertexAttributeBuffer& buffer);

		/**
		* Binds this VAO for rendering.
		*/
		void bind();

		/**
		* Unbinds the VAO.
		*/
		void unbind();

	private:

		using VertexBufferBindings = std::unordered_map<unsigned int, const VertexAttributeBuffer*>;
		using ContextSpecificStateMap = std::unordered_map<GLContext, GLuint>;
		ContextSpecificStateMap	mContextSpecificState;				// The per-context vertex array state

		/**
		 * Maps a buffer to a specific pointer index in the vertex array object
		 * This index is important when enabling the VAO for drawing with a shader
		 * The index can be bound to a vertex shader attribute location using a name
		 * For that reason it's important to associate an id with every added buffer
		 */
		VertexBufferBindings	mBindings;				
	};
}
