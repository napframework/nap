#pragma once

// Local Includes
#include "ngltypes.h"
#include "nglutils.h"

// External Includes
#include <GL/glew.h>
#include <stdint.h>

namespace opengl
{
	/**
	 * Defines settings for a vertex buffer
	 * 
	 * Convenience wrapper to be used in conjunction with a vertex buffer
	 */
	struct VertexBufferSettings
	{
		VertexBufferSettings() = default;
		VertexBufferSettings(GLenum type, unsigned int components, unsigned int verts, GLenum usage = GL_STATIC_DRAW) :
			mType(type),
			mComponents(components),
			mVerts(verts)				{ }

		unsigned int	mComponents  = 0;			// defines the number of components (3 for color, 2 for uv's etc)
		GLenum			mType  = GL_INVALID_ENUM;	// defines the internal GL type of the buffer
		unsigned int	mVerts = 0;					// defines the number of points in the buffer
		GLenum			mUsage = GL_STATIC_DRAW;	// defines the expected usage pattern of the data storage: https://www.opengl.org/sdk/docs/man4/html/glBufferData.xhtml

		/**
		 * @return total length of array: the number of verts multiplied by the number of components
		 */
		std::size_t		getLength() const;

		/**
		 * @return the total size in bytes of these settings when used by the buffer
		 */
		std::size_t		getSize() const;

		/**
		 * @return if the settings are valid for the buffer to use
		 */
		bool			isValid() const;
	};


	/**
	 * VertexBufferBase
	 *
	 * Defines a vertex buffer on the GPU that is associated with a single set of vertex data
	 * Vertex data is arbitrary vertex data such as position, uv, color etc.
	 * This object does not manage or owns any data
	 */
	class VertexBuffer
	{
	public:
		VertexBuffer() = default;
		VertexBuffer(const VertexBufferSettings& settings) : mSettings(settings) { };
		virtual ~VertexBuffer();

		/**
		 * Allocates the buffer on the GPU
		 * Always call init after creation otherwise subsequent calls will fail
		 */
		void init();

		/**
		 * @return if the buffer is allocated on the GPU
		 */
		bool isAllocated() const								{ return mId != 0; }

		/**
		 * @return the id associated with this buffer on the GPU
		 */
		GLuint getId() const									{ return mId; }

		/**
		 * binds this vertex buffer on the GPU for subsequent GPU calls
		 */
		bool bind();

		/**
		 * unbinds this vertex buffer on the GPU for subsequent GPU calls
		 */
		bool unbind();

		/**
		 * @return the settings associated with this buffer
		 */
		const VertexBufferSettings& getSettings() const			{ return mSettings; }

		/**
		 * @return the buffer OpenGL type, INVALID_ENUM if not specified
		 */
		 virtual GLenum getType() const							{ return mSettings.mType; }

		/**
		 * @return the number of components associated with the buffer
		 */
		unsigned int getComponentCount() const					{ return mSettings.mComponents; }

		/**
		 * @return the number of vertices
		 */
		unsigned int getVertCount() const						{ return mSettings.mVerts; }

		/**
		 * @return the total length of the buffer (verts * components) 
		 */
		size_t getLength() const								{ return mSettings.getLength(); }

		/**
		 * @return the total data size of the buffer in bytes
		 */
		size_t getSize() const									{ return mSettings.getSize(); }

		/**
		 * @return the expected usage pattern of the data storage
		 */
		GLenum getUsage() const									{ return mSettings.mUsage; }

		/**
		 * Uploads data to the GPU based on the settings provided
		 * Settings need to match data size container, otherwise the application will crash
		 * The pointer to the block of data in memory (not GPU) is not stored
		 * This function binds the buffer before uploading the data
		 * @param settings the settings that define the size and type of the buffer
		 * @param data pointer to the block of data that needs to be uploaded
		 */
		virtual void setData(const VertexBufferSettings& settings, void* data);

		/**
		 * Uploads data to the GPU based on the settings provided
		 * Settings need to match data size container, otherwise the application will crash
		 * The pointer to the block of data in memory (not GPU) is not stored
		 * This function binds the buffer before uploading the data
		 * @param type OpenGL associated data type
		 * @param components number of components for every vert, say 3 for position, 2 for uv etc.
		 * @param usage the expected usage pattern of the data storage
		 * @param data pointer to the block of data the is uploaded
		 */
		virtual void setData(GLenum type, unsigned int components, unsigned int verts, GLenum usage, void* data);

		/**
		 * Uploads data to the GPU
		 *
		 * Note that the settings associated with this buffer need to match the size of the data block
		 * If they don't match the application could crash
		 * The pointer to the data is not stored
		 * This function binds the buffer before uploading the data
		 * @param data pointer to the block of data that needs to be uploaded
		 */
		virtual void setData(void* data);

	protected:
		// Buffer GPU ID
		GLuint					mId   = 0;

		// Settings associated with this buffer
		VertexBufferSettings	mSettings;
	};
	
	//////////////////////////////////////////////////////////////////////////

	/**
	 * TypedVertexBuffer
	 *
	 * Specialization of a VertexBuffer
	 * Acts as a simple wrapper class that defines it's own associated OpenGL buffer type
	 */
	template <typename T>
	class TypedVertexBuffer : public VertexBuffer
	{
	public:
		TypedVertexBuffer();
		TypedVertexBuffer(const VertexBufferSettings& settings);

		/**
		 * Every typed buffer is associated with a certain OpenGL buffer type
		 * This method needs to be implemented for every specialized type of buffer
		 */
		virtual GLenum getType() const override;


		/**
		 * Uploads data to the GPU
		 *
		 * Note that the settings associated with this buffer need to match the size of the data block
		 * @param data pointer to the block of data that needs to be uploaded
		 */
		void setData(T* data);


		/**
		* Uploads data to the GPU based on the settings provided
		* Settings need to match data size container, otherwise the application will crash
		* The pointer to the block of data in memory (not GPU) is not stored
		* This function binds the buffer before uploading the data
		* @param settings the settings that define the size, type is defined by this class
		* @param data pointer to the block of data that needs to be uploaded
		*/
		void setData(const VertexBufferSettings& settings, T* data);


		/**
		 * Uploads data to the GPU
		 *
		 * Note that the settings associated with this buffer need to match the size of the data block
		 * @param components number of components for every vert, say 3 for position, 2 for uv etc.
		 * @param usage the expected usage pattern of the data storage
		 * @param data pointer to the block of data the is uploaded
		 */
		void setData(unsigned int components, unsigned int verts, GLenum usage, T* data);


	private:
		// Disable base class functionality
		void setData(GLenum type, unsigned int components, unsigned int verts, GLenum usage, void* data) override		{ }
		void setData(void* data) override																				{ }
		void setData(const VertexBufferSettings& settings, void* data) override											{ }
	};


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template <typename T>
	opengl::TypedVertexBuffer<T>::TypedVertexBuffer(const VertexBufferSettings& settings) : VertexBuffer(settings)
	{
		// Override type
		mSettings.mType = getType();
	}


	// Default constructor
	template <typename T>
	opengl::TypedVertexBuffer<T>::TypedVertexBuffer()
	{
		// Override type
		mSettings.mType = getType();
	}

	
	// Uploads the data block
	template <typename T>
	void opengl::TypedVertexBuffer<T>::setData(T* data)
	{
		VertexBuffer::setData(static_cast<void*>(data));
	}


	// Updates settings and uploads data block
	template <typename T>
	void opengl::TypedVertexBuffer<T>::setData(unsigned int components, unsigned int verts, GLenum usage, T* data)
	{
		mSettings.mComponents = components;
		mSettings.mVerts = verts;
		mSettings.mUsage = usage;
		TypedVertexBuffer<T>::setData(data);
	}


	// Updates settings and uploads data
	template <typename T>
	void opengl::TypedVertexBuffer<T>::setData(const VertexBufferSettings& settings, T* data)
	{
		if (settings.mType != getType())
		{
			opengl::printMessage(opengl::MessageType::WARNING, "trying to upload vertex data with an inappropriate data type!");
			opengl::printMessage(opengl::MessageType::WARNING, "data type does not match vertex buffer data type!");
		}
		VertexBuffer::setData(settings, static_cast<void*>(data));
	}


	//////////////////////////////////////////////////////////////////////////
	// Buffer Typedefs
	//////////////////////////////////////////////////////////////////////////
	using FloatVertexBuffer  =	TypedVertexBuffer<float>;
	using IntVertexBuffer	 =	TypedVertexBuffer<int>;
	using ByteVertexBuffer	 =	TypedVertexBuffer<int8_t>;
	using DoubleVertexBuffer =	TypedVertexBuffer<double>;
}
