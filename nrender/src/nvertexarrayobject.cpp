#include "nvertexarrayobject.h"
#include <nsdlgl.h>
#include <assert.h>

namespace opengl
{
	VertexArrayObject::~VertexArrayObject()
	{
		// At this point, all context specific state must be destroyed
		assert(mContextSpecificState.empty());
	}


	void VertexArrayObject::destroy(opengl::GLContext context)
	{
		ContextSpecificStateMap::iterator pos = mContextSpecificState.find(context);
		if (pos != mContextSpecificState.end())
		{
			glDeleteVertexArrays(1, &pos->second);
			mContextSpecificState.erase(pos);
		}
	}


	void VertexArrayObject::bind()
	{
		void* context = opengl::getCurrentContext();

		ContextSpecificStateMap::const_iterator state = mContextSpecificState.find(context);
		if (state == mContextSpecificState.end())
		{
			// No state found, generate VAO and bind buffers
			GLuint id;
			glGenVertexArrays(1, &id);

			mContextSpecificState.emplace(std::make_pair(context, id));

			glBindVertexArray(id);

			for (const auto& binding : mBindings)
			{
				const VertexAttributeBuffer& buffer = *binding.second;

				buffer.bind();
				glEnableVertexAttribArray(binding.first);
				switch (buffer.getType())
				{
					case GL_BYTE:
					case GL_UNSIGNED_BYTE:
					case GL_INT:
					case GL_UNSIGNED_INT:
					case GL_SHORT:
					case GL_UNSIGNED_SHORT:
					{
						glVertexAttribIPointer(binding.first, buffer.getNumComponents(), buffer.getType(), 0, 0);
						break;
					}
					default:
					{
						glVertexAttribPointer(binding.first, buffer.getNumComponents(), buffer.getType(), GL_FALSE, 0, 0);
						break;
					}


				}
				buffer.unbind();
			}
		}
		else
		{
			glBindVertexArray(state->second);
		}
	}


	void VertexArrayObject::unbind()
	{
		glBindVertexArray(0);
	}


	//Adds and binds a vertex buffer to this vertex array object
	void VertexArrayObject::addVertexBuffer(unsigned int index, const VertexAttributeBuffer& buffer)
	{
		// Vertex buffer cannot be added if we have already created our internal
		// structures for VAOs. We first add all vertex buffer, then we render.
		// (no dynamic adding of vertex buffers).
		assert(mContextSpecificState.empty());
		assert(mBindings.find(index) == mBindings.end());

		// Add index
		mBindings.emplace(std::make_pair(index, &buffer));
	}
}
