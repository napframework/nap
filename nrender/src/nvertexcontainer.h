#pragma once

// Local Includes
#include "nvertexbuffer.h"

// External Includes
#include <memory>

namespace opengl
{
	/**
	 * VertexContainer
	 *
	 * Manages a set of vertex data in CPU memory that can be uploaded to the GPU
	 * This object owns that vertex data. Vertex data is data such as uv's, color etc.
	 *
	 * By default no vertex buffer is associated with a container, if the user
	 * requests a vertex buffer a new one is created based on the container settings
	 * This buffer is also tied to this object and destroyed when the container is destroyed
	 *
	 * Use this object to store vertex data and get a hardware handle to that data on the GPU
	 * For fine grained GPU vertex control use the VertexBuffer
	 * The buffer managed by this container is defined static by default and can't be changed for now
	 */
	class VertexContainer
	{
	public:
		// Default constructor
		VertexContainer() = default;

		// Don't allow copy, TODO: implement copy
		VertexContainer(const VertexContainer& other) = delete;
		VertexContainer& operator=(const VertexContainer& other) = delete;
		
		/**
		 * Construct the vertex container using vertex buffer settings
		 */
		VertexContainer(const VertexAttributeBufferSettings& settings) : mSettings(settings)		{ }

		/**
		 * Destroys allocated buffer data
		 */
		virtual ~VertexContainer();

		/**
		 * Allocate CPU memory based on current vertex buffer settings
		 * If no settings have been provided this call will fail
		 * @return if allocation was successful
		 */
		virtual bool allocateMemory();

		/**
		 * Allocate CPU memory based on the settings provided
		 * If the settings are invalid no memory will be allocated
		 * @param type the OpenGL data type associated with this container
		 * @param components number of buffer components, say 3 for position, 2 for uv etc.
		 * @param verts the number of vertices to allocate memory for
		 * @return if allocation was successful
		 */
		virtual bool allocateMemory(GLenum type, unsigned int components, unsigned int verts);

		/**
		 * Copies data from source in to this vertex container
		 * Note that data set previously will be cleared
		 * If no data is associated with the container new memory is allocated
		 * If the container does not have the right settings before calling copy the application could crash
		 * @param source data to copy
		 * @return if the copy was successful or not
		 */
		virtual bool copyData(const void* source);

		/**
		 * Copies data from source in to this vertex container
		 * Note that data set previously will be cleared
		 * If no data is associated with the container new memory is allocated
		 * If the container does not have the right settings before calling copy the application could crash
		 * Note that this call will update CPU memory, if  a GPU buffer has been created those settings will be invalidated
		 * To make sure the GPU buffer is in sync with local memory -> call sync()
		 * @param type the OpenGL data type associated with this container
		 * @param components number of buffer components, say 3 for position, 2 for uv etc.
		 * @param verts the number of vertices to allocate memory for
		 * @param source data to copy
		 * @return if copy was successful
		 */
		virtual bool copyData(GLenum type, unsigned int components, unsigned int verts, const void* source);

		/**
		 * Sets data to be associated with this vertex container
		 * Note that the data that is set will be owned by this container
		 * The data needs to match the settings associated with this container
		 * Failure to match size requirements will lead to unexpected behavior or crashes
		 * Use with caution!
		 */
		virtual void setData(void* data);

		/**
		 * Returns data associated with this vertex container
		 */
		virtual const void* getData() const				{ return mData; }

		/**
		 * Returns if there is vertex data associated with the container
		 * This is valid when data has been copied or set
		 */
		bool hasData() const							{ return mData != nullptr; }

		/**
		 * Deletes all CPU data associated with the container
		 * Note that this does not delete the GPU buffer associated with this container!
		 */
		void clear();

		/**
		 * @return the size in bytes of the data associated with the container
		 */
		size_t getSize() const;

		/**
		 * @return the total length of the buffer, not scaled by byte size of associated data type 
		 */
		size_t getLength() const;

		/**
		 * @return the total number of vertices
		 */
		unsigned int getVertCount() const						{ return mSettings.mVerts; }

		/**
		* @return the number of components associated with the buffer
		*/
		unsigned int getComponentCount() const					{ return mSettings.mComponents; }

		/**
		* @return the buffer OpenGL type, INVALID_ENUM if not specified
		*/
		virtual GLenum getType() const							{ return mSettings.mType; }

		/**
		* @return the settings associated with this buffer
		*/
		const VertexAttributeBufferSettings& getSettings() const			{ return mSettings; }

		/**
		 * @return a GPU vertex buffer that holds the data associated with this container, nullptr if buffer can't be created
		 * This function will create a buffer if no buffer has been created yet
		 * If there is data associated with this container that data will be uploaded to the GPU when the buffer is created
		 * If no data is set but the buffer is created call update to upload new data
		 * When the data in the container changes (settings), make sure to change the settings for the buffer
		 * When the CPU data has changed, call update to upload the changes on to the GPU
		 * The returned buffer will be owned by this object and is deleted on object destruction
		 */
		VertexAttributeBuffer* getVertexBuffer();

		/**
		 * Uploads the data on to the GPU without synchronizing settings
		 * The update will be based on the settings associated with this container
		 * Data that does not match buffer settings will result in undefined behavior, possibly a crash;
		 * Note that if no data is set or the container settings are invalid, this call will fail
		 * If there's no vertex buffer one will be created by this call
		 */
		void update();

		/**
		 * Updates the GPU settings based on current container settings and pushes data on to the GPU
		 * Useful when a new set of data is copied with different settings and the GPU component is out of sync
		 * This call will apply settings and push a GPU update
		 * Note that if no data is set or the container settings are invalid, this call will fail
		 */
		void sync();

	protected:
		// Buffer Settings
		VertexAttributeBufferSettings mSettings;

		// Buffer Data (CPU)
		void* mData = nullptr;

		// Vertex Buffer (GPU)
		std::unique_ptr<VertexAttributeBuffer> mGPUBuffer = nullptr;

	private:
		// Helper function top create a new GPU bound vertex buffer
		std::unique_ptr<VertexAttributeBuffer> createVertexBuffer();
	};


	//////////////////////////////////////////////////////////////////////////
	
	/**
	 * TypedVertexContainer
	 *
	 * A specialization of a vertex container associated with a type
	 * Allows for easy retrieval and setting of data where the
	 * typed container holds it's own GL_TYPE. 
	 * Every specialization of this type needs to implement the getType() method
	 */
	template<typename T>
	class TypedVertexContainer : public VertexContainer
	{
	public:
		// Constructor
		TypedVertexContainer();
		TypedVertexContainer(unsigned int components, unsigned int verts);

		/**
		 * Needs to be implemented by every specialized type
		 */
		virtual GLenum getType() const override;

		/**
		* Allocate CPU memory based on the settings provided
		* If the settings are invalid no memory will be allocated
		* @param components number of buffer components, say 3 for position, 2 for uv etc.
		* @param verts the number of vertices to allocate memory for
		* @return if allocation was successful
		*/
		bool allocateMemory(unsigned int components, unsigned int verts);

		/**
		* Copies data from source in to this vertex container
		* Note that data set previously will be cleared
		* If no data is associated with the container new memory is allocated
		* If the container does not have the right settings before calling copy the application could crash
		* @param source data to copy
		* @return if the copy was successful or not
		*/
		bool copyData(const T* source);

		/**
		* Copies data from source in to this vertex container
		* Note that data set previously will be cleared
		* If no data is associated with the container new memory is allocated
		* If the container does not have the right settings before calling copy the application could crash
		* Note that this call will update CPU memory, if  a GPU buffer has been created those settings will be invalidated
		* To make sure the GPU buffer is in sync with local memory -> call sync()
		* @param components number of buffer components, say 3 for position, 2 for uv etc.
		* @param verts the number of vertices to allocate memory for
		* @param source data to copy
		* @return if copy was successful
		*/
		bool copyData(unsigned int components, unsigned int verts, const T* source);

		/**
		* Sets data to be associated with this vertex container
		* Note that the data that is set will be owned by this container
		* The data needs to match the settings associated with this container
		* Failure to match size requirements will lead to unexpected behavior or crashes
		* Use with caution!
		*/
		void setData(T* data);

		/**
		* Returns data associated with this vertex container
		*/
		virtual const T* getTypedData() const;

	private:
		// override and hide base class functionality																
		virtual bool allocateMemory(GLenum type, unsigned int components, unsigned int verts) override			{ return false; }
		virtual bool copyData(GLenum type, unsigned int components, unsigned int verts, const void* source) override	{ return false; }
		virtual bool copyData(const void* source) override															{ return false; }
		virtual void setData(void* data) override																{ }

		// utilities
		bool updateSettingsIfValid(unsigned int components, unsigned int verts);
	};

	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	// Forces internal type
	template<typename T>
	opengl::TypedVertexContainer<T>::TypedVertexContainer()
	{
		mSettings.mType = getType();
	}


	// Stores settings and forces internal type
	template<typename T>
	opengl::TypedVertexContainer<T>::TypedVertexContainer(unsigned int components, unsigned int verts)
	{
		mSettings.mComponents = components;
		mSettings.mVerts = verts;
		mSettings.mType = getType();
	}
 

	// Allocates memory for the typed vertex container
	template<typename T>
	bool opengl::TypedVertexContainer<T>::allocateMemory(unsigned int components, unsigned int verts)
	{
		// Make sure new settings are valid
		if (!updateSettingsIfValid(components, verts))
			return false;

		// Allocate
		return VertexContainer::allocateMemory();
	}


	// Copies data in to container, note that settings have to be valid for this to work
	template<typename T>
	bool opengl::TypedVertexContainer<T>::copyData(const T* source)
	{
		return VertexContainer::copyData(static_cast<const void*>(source));
	}


	// Copies data in to container, note that components and verts > 0
	template<typename T>
	bool opengl::TypedVertexContainer<T>::copyData(unsigned int components, unsigned int verts, const T* source)
	{
		if (!updateSettingsIfValid(components, verts))
			return false;
		return VertexContainer::copyData(static_cast<const void*>(source));
	}


	// Update internal settings if valid
	template<typename T>
	bool opengl::TypedVertexContainer<T>::updateSettingsIfValid(unsigned int components, unsigned int verts)
	{
		if (components == 0)
		{
			printMessage(MessageType::ERROR, "unable to update vertex container settings, invalid number of components");
			return false;
		}

		if (verts == 0)
		{
			printMessage(MessageType::ERROR, "unable to update vertex container settings, invalid number of vertices");
			return false;
		}

		mSettings.mComponents = components;
		mSettings.mVerts = verts;
		return true;
	}


	// Sets data to be managed by this container, use with caution! 
	template<typename T>
	void opengl::TypedVertexContainer<T>::setData(T* data)
	{
		VertexContainer::setData(static_cast<void*>(data));
	}

	
	// Returns data associated with this container
	template<typename T>
	const T* opengl::TypedVertexContainer<T>::getTypedData() const
	{
		return static_cast<const T*>(VertexContainer::getData());
	}

	//////////////////////////////////////////////////////////////////////////
	// Container Typedefs
	//////////////////////////////////////////////////////////////////////////
	using FloatVertexContainer  = TypedVertexContainer<float>;
	using IntVertexContainer    = TypedVertexContainer<int>;
	using ByteVertexContainer   = TypedVertexContainer<int8_t>;
	using DoubleVertexContainer = TypedVertexContainer<double>;

} // opengl
