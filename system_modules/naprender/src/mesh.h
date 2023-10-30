/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "vertexattribute.h"

// External includes
#include <gpumesh.h>
#include <memory>
#include <utility/dllexport.h>
#include <rtti/object.h>
#include <rtti/objectptr.h>
#include <nap/numeric.h>
#include <rtti/factory.h>
#include <nap/numeric.h>

namespace nap
{
	class RenderService;
	class Core;

	/**
	 * Topology of the mesh
	 */
	enum class EDrawMode : int32
	{
		Points			= 0,				///< Interpret the vertex data as single points
		Lines			= 1,				///< Interpret the vertex data as individual lines
		LineStrip		= 2,				///< Interpret the vertex data as a single connected line
		Triangles		= 3,				///< Interpret the vertex data as a set of triangles
		TriangleStrip	= 4,				///< Interpret the vertex data as a strip of triangles
		TriangleFan		= 5,				///< Interpret the vertex data as a fan of triangles
		Unknown			= 0x7FFFFFFF,		///< Invalid vertex interpretation
	};


	/**
	 * Triangle cull modes
	 */
	enum class ECullMode : int32
	{
		None			= 0,				///< No culling
		Front			= 1,				///< Cull front facing triangles
		Back			= 2,				///< Cull back facing triangles
		FrontAndBack	= 3					///< Cull front and back facing triangles
	};


	/**
	 * Polygon modes
	 */
	enum class EPolygonMode : int32
	{
		Fill			= 0,				///< Polygons are interpreted and rendered using the specified 'EDrawMode'
		Line			= 1,				///< Polygon edges are drawn as line segments.
		Point			= 2					///< Polygon vertices are drawn as points.
	};


	//////////////////////////////////////////////////////////////////////////
	// MeshShape
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Contains a list of indices that describe which particular part of the nap::MeshInstance should be drawn.
	 * Every nap::MeshInstance contains at least one shape.
	 */
	struct NAPAPI MeshShape
	{
		/**
		 * @return The number of indices in this shape
		 */
		int getNumIndices() const										{ return mIndices.size(); }

		/**
		* Clears the list of indices.
		* Call either before init() or call update() to reflect the changes in the GPU buffer.
		*/
		void clearIndices()												{ mIndices.clear(); }

		/**
		* Reserves CPU memory for index list. GPU memory is reserved after update() is called.
		* @param numIndices Amount of indices to reserve.
		*/
		void reserveIndices(size_t numIndices);

		/**
		* Adds a list of indices to the index CPU buffer.
		* Call either before init() or call update() to reflect the changes in the GPU buffer.
		* @param indices: array of indices to add.
		* @param numIndices: size of the array.
		*/
		void setIndices(uint32* indices, int numIndices);

		/**
		 * @return The index list for this shape
		 */
		const std::vector<uint32>& getIndices() const					{ return mIndices; }

		/**
		 * @return The index list for this shape
		 */
		std::vector<uint32>& getIndices()								{ return mIndices; }

		/**
		 * Adds a number of indices to the existing indices in the index CPU buffer. Use setIndices to replace
		 * the current indices with a new set of indices.
		 * Call either before init() or call update() to reflect the changes in the GPU buffer.
		 * @param indices List of indices to update.
		 * @param numIndices Number of indices in the list.
		 */
		void addIndices(uint32* indices, int numIndices);

		/**
		 * Adds a single index to the index CPU buffer. Use setIndices to add an entire list of indices.
		 * Call either before init() or call update() to reflect the changes in the GPU buffer.
		 * @param index Index to add.
		 */
		void addIndex(uint32 index)										{ mIndices.emplace_back(index); }

		/**
		 * Array subscript overload.
		 * @param index lookup index
		 * @return value at the given index
		 */
		uint32& operator[](std::size_t index)							{ return mIndices[index]; }
		
		/**
		 * Const array subscript overload.
		 * @param index lookup index
		 * @return value at the given index
		 */
		const uint32& operator[](std::size_t index) const				{ return mIndices[index]; }

		std::vector<uint32>			mIndices;		///< Property: 'Indices' into the mesh's vertex data
	};


	//////////////////////////////////////////////////////////////////////////
	// MeshProperties
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Helper struct for data that is common between MeshInstance and Mesh.
	 * Templatized as the ownership for vertex attributes is different between a MeshInstance and a Mesh:
	 *		- MeshInstance has ownership over the attributes through unique_ptrs.
	 *		- Mesh uses ObjectPtrs as the ResourceManager has ownership.
	 * 
	 * The difference in ownership is important as json-based data is owned by the resourceManager, whereas
	 * mesh data does not always originate from a json-based source, as is the case for procedural meshes
	 * and binary-based loading of meshes.
	 */
	template<typename VERTEX_ATTRIBUTE_PTR>
	struct MeshProperties
	{
		using VertexAttributeList = std::vector<VERTEX_ATTRIBUTE_PTR>;

		int						mNumVertices = 0;					///< Property: 'NumVertices' number of mesh vertices
		EMemoryUsage			mUsage = EMemoryUsage::Static;		///< Property: 'Usage' GPU memory usage
		EDrawMode				mDrawMode = EDrawMode::Triangles;	///< Property: 'DrawMode' The draw mode that should be used to draw the shapes
		ECullMode				mCullMode = ECullMode::Back;		///< Property: 'CullMode' The triangle cull mode to use
		EPolygonMode			mPolygonMode = EPolygonMode::Fill;	///< Property: 'PolygonMode' The polygon mode to use, fill is always available and should be the default
		VertexAttributeList		mAttributes;						///< Property: 'Attributes' vertex attributes
		std::vector<MeshShape>	mShapes;							///< Property: 'Shapes' list of managed shapes
	};

	// ObjectPtr based mesh properties, used in serializable Mesh format (json/binary)
	using RTTIMeshProperties = MeshProperties<rtti::ObjectPtr<BaseVertexAttribute>>;


	//////////////////////////////////////////////////////////////////////////
	// MeshInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Runtime version of a mesh. MeshInstance holds CPU data and can convert this data into a GPUMesh. 
	 * 
	 * A MeshInstance can be created in two ways:
	 *		1) Manually allocate a MeshInstance object, add attributes to it, and call the regular init() on it. 
	 *		   This is useful for creation of procedural meshes.
	 * 
	 *      2) Initialize a MeshInstance object through the init() function that takes an RTTIMeshProperties struct.
	 *         This is intended for situations where the MeshInstance is created from an RTTI based object (either 
	 *         rtti-json or rtti-binary).
	 * 
	 * After the initial init() call, the GPU attributes are created and the mesh is updated. This means that the
	 * contents of the vertex attributes and the index buffer is uploaded to the GPU. If the contents of the vertex 
	 * attributes are filled in before calling init(), the mesh is fully up to date.
	 *
	 * It is also possible to update the mesh dynamically. The vertex attribute contents should be updated, followed by
	 * an update() call.
	 */
	class NAPAPI MeshInstance
	{
		RTTI_ENABLE()
	public:
		// Default constructor
		MeshInstance(RenderService& renderService);

		// destructor
		virtual ~MeshInstance();

		/**
 		 * Builds a GPU mesh from the CPU mesh. Attribute information should be filled in prior to calling init().
		 * @param errorState Contains error information if an error occurred.
		 * @return True if succeeded, false on error.
		 */
		bool init(utility::ErrorState& errorState);

		/**
		 * Clones the RTTI based data but does not build a GPU mesh from it. Call init to upload
		 * the cloned data to the GPU. The cloned data is owned by MeshInstance
		 * @param meshProperties The RTTI mesh properties to clone into the mesh instance.
		 */
		void copyMeshProperties(RTTIMeshProperties& meshProperties);

		/**
		 * @return the mesh that can be drawn to screen or buffer
		 */
		GPUMesh& getGPUMesh() const;

		/**
		 * Finds vertex attribute.
		 * @param id The name of the vertex attribute. For predefined vertex attributions like position, color etc, use the various MeshInstance::VertexAttributeIDs.
		 * @return Type safe vertex attribute if found, nullptr if not found or if there was a type mismatch.
		 */
		template<typename T>
		VertexAttribute<T>* findAttribute(const std::string& id);

		/**
		 * Finds vertex attribute.
		 * @param id The name of the vertex attribute. For predefined vertex attributions like position, color etc, use the various MeshInstance::VertexAttributeIDs.
		 * @return Type safe vertex attribute if found, nullptr if not found or if there was a type mismatch.
		 */
		template<typename T>
		const VertexAttribute<T>* findAttribute(const std::string& id) const;

		/**
		 * Gets vertex attribute.
		 * @param id The name of the vertex attribute. For predefined vertex attributions like position, color etc, use the various MeshInstance::VertexAttributeIDs.
		 * @return Type safe vertex attribute. If not found or in case there is a type mismatch, the function asserts.
		 */
		template<typename T>
		VertexAttribute<T>& getAttribute(const std::string& id);

		/**
		 * Gets vertex attribute.
		 * @param id The name of the vertex attribute. For predefined vertex attributions like position, color etc, use the various MeshInstance::VertexAttributeIDs.
		 * @return Type safe vertex attribute. If not found or in case there is a type mismatch, the function asserts.
		 */
		template<typename T>
		const VertexAttribute<T>& getAttribute(const std::string& id) const;

		/**
		 * Gets a vertex attribute or creates it if it does not exist. In case the attribute did exist, but with a different type, the function asserts.
		 * @param id The name of the vertex attribute. For predefined vertex attributions like position, color etc, use the various MeshInstance::VertexAttributeIDs.
		 * @return Type safe vertex attribute. 
		 */
		template<typename T>
		VertexAttribute<T>& getOrCreateAttribute(const std::string& id);

		/**
	 	 * Reserves CPU memory for the given amount of vertices. All associated vertex attributes are affected.
		 * GPU memory is reserved after update() is called.
		 * @param numVertices amount of vertices to reserve memory for.
		 */
		void reserveVertices(size_t numVertices);

		/**
		 * Sets number of vertices. The amount of elements for each vertex buffer should be equal to
		 * the amount of vertices in the mesh.
		 * @param numVertices: amount of vertices in the mesh.
		 */
		void setNumVertices(int numVertices)									{ mProperties.mNumVertices = numVertices; }

		/**
		 * @return the total number of vertices associated with this mesh
		 */
		int getNumVertices() const												{ return mProperties.mNumVertices; }

		/**
		 * @return The number of shapes contained in this mesh
		 */
		int getNumShapes() const												{ return mProperties.mShapes.size(); }

		/**
		* @return Set the topology of this mesh (triangle list, strip, lines etc).
		*/
		void setDrawMode(EDrawMode mode)										{ mProperties.mDrawMode = mode; }

		/**
		 * @return The topology of this mesh (triangle list, strip, lines etc).
		 */
		EDrawMode getDrawMode() const											{ return mProperties.mDrawMode; }

		/**
		 * @param mode the cull mode of this mesh (front, back etc.)
		 */
		void setCullMode(ECullMode mode)										{ mProperties.mCullMode = mode; }

		/**
		 * @return the cull mode of this mesh (front, back etc.)
		 */
		ECullMode getCullMode() const											{ return mProperties.mCullMode; }

		/**
		 * Sets the polygon mode of this mesh to use (fill, line or point).
		 * Logs a warning if the requested mode is not supported.
		 * To ensure a specific mode is supported call: RenderService::getPolygonModeSupported().
		 * @param mode the polygon mode to use
		 */
		void setPolygonMode(EPolygonMode mode);

		/**
		 * @return the polygon mode of this mesh
		 */
		EPolygonMode getPolygonMode() const										{ return mProperties.mPolygonMode; }							

		/**
		 * Get the shape at the specified index
		 * @param index The index of the shape to get (between 0 and getNumShapes())
		 * @return The shape
		 */
		MeshShape& getShape(int index)											{ return mProperties.mShapes[index]; }

		/**
		 * Get the shape at the specified index
		 * @param index The index of the shape to get (between 0 and getNumShapes())
		 * @return The shape
		 */
		const MeshShape& getShape(int index) const								{ return mProperties.mShapes[index]; }

		/**
		 * Create and add a new shape to this mesh. The returned shape is uninitialized; it is up to the client to initialize as needed
		 * @return The new shape
		 */
		MeshShape& createShape();

		/**
		 * Set the usage for this mesh. Note that it only makes sense to change this before init is called, 
		 * changing it after init will not have any effect.
		 */
		void setUsage(EMemoryUsage inUsage)										{ mProperties.mUsage = inUsage; }

		/**
		 * @return how this mesh is used at runtime
		 */
		EMemoryUsage getUsage() const											{ return mProperties.mUsage; }

		/**
		 * Pushes all CPU vertex buffers to the GPU. Note that update() is called during init(),
		 * so this is only required if CPU data is modified after init().
		 * Only update the mesh when 'Usage' is set to 'DynamicWrite', an assert is triggered otherwise.
		 * If there is a mismatch between vertex buffer, an error will be returned.
		 * @param errorState Contains error information if an error occurred.
		 * @return True if succeeded, false on error.		 
		 */
		bool update(utility::ErrorState& errorState);

		/**
		 * Push one specific CPU vertex buffer to the GPU.
		 * Use this when updating only specific vertex attributes at run-time.
		 * Only update the mesh when 'Usage' is set to 'DynamicWrite', an assert is triggered otherwise.
		 * If there is a mismatch between the vertex buffer an error will be returned
		 * @param attribute the attribute to synchronize.
		 * @param errorState contains the error when synchronization fails.
		 * @return if the update succeeded or not.
 		 */
		bool update(nap::BaseVertexAttribute& attribute, utility::ErrorState& errorState);

	protected:
		bool initGPUData(utility::ErrorState& errorState);

	private:
		RenderService&											mRenderService;			///< Required reference to the render service
		MeshProperties<std::unique_ptr<BaseVertexAttribute>>	mProperties;			///< CPU mesh data
		std::unique_ptr<GPUMesh>								mGPUMesh;				///< GPU mesh
		bool													mInitialized = false;	///< If the instance is initialized
	};


	//////////////////////////////////////////////////////////////////////////
	// IMesh
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Mesh resource interface. Derive from this class to implement your own serializable mesh resource.
	 * Every IMesh should be able to construct and return a nap::MeshInstance().
	 * The instance is rendered and can be updated / modified at runtime.
	 */
	class IMesh : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * @return the mesh instance
		 */
		virtual MeshInstance& getMeshInstance() = 0;

		/**
		 * @return the mesh instance
		 */
		virtual const MeshInstance& getMeshInstance() const = 0;
	};


	//////////////////////////////////////////////////////////////////////////
	// Mesh
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Serializable Mesh. This mesh can be used to either load from json or binary format.
	 * Notice that the rtti properties that are deserialized should remain constant, like 
	 * any other resource. To alter mesh data dynamically, use the MeshInstance that is returned from this
	 * object.
	 *
	 * The RTTI data in the Mesh can conveniently be used as a source to alter the MeshInstance's 
	 * contents. For instance, it is possible to copy the original data from the Mesh into the
	 * MeshInstance while making some modifications.
	 */
	class Mesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		/**
		 * Constructor used during serialization.
		 */
		Mesh() = default;

		/**
		 * Constructor used at runtime.
		 */
		Mesh(Core& core);

		/**
		 * Initialized the mesh instance.
		 * @param errorState Contains error information if an error occurred.
		 * @return True if succeeded, false on error.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return MeshInstance as created during init().
		 */
		virtual MeshInstance& getMeshInstance()	override;

		/**
		 * @return MeshInstance as created during init().
		 */
		virtual const MeshInstance& getMeshInstance() const	override;

		/**
		 * Finds vertex attribute.
		 * @param id The name of the vertex attribute. For predefined vertex attributions like position, color etc, use the various MeshInstance::VertexAttributeIDs.
		 * @return Type safe vertex attribute if found, nullptr if not found or if there was a type mismatch.
		 */
		template<typename T>
		const VertexAttribute<T>* FindAttribute(const std::string& id) const;

		/**
		 * Gets vertex attribute.
		 * @param id The name of the vertex attribute. For predefined vertex attributions like position, color etc, use the various MeshInstance::VertexAttributeIDs.
		 * @return Type safe vertex attribute. If not found or in case there is a type mismatch, the function asserts.
		 */
		template<typename T>
		const VertexAttribute<T>& GetAttribute(const std::string& id) const;

		RTTIMeshProperties	mProperties;				///< Property: 'Properties' RTTI mesh CPU data

	private:
		std::unique_ptr<MeshInstance> mMeshInstance;	///< Runtime mesh instance
		RenderService* mRenderService = nullptr;		///< Render service
	};


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	nap::VertexAttribute<T>* nap::MeshInstance::findAttribute(const std::string& id)
	{
		for (auto& attribute : mProperties.mAttributes)
			if (attribute->mAttributeID == id)
				return rtti_cast<VertexAttribute<T>>(attribute.get());
		return nullptr;
	}

	template<typename T>
	const VertexAttribute<T>* nap::MeshInstance::findAttribute(const std::string& id) const
	{
		for (auto& attribute : mProperties.mAttributes)
		{
			if (attribute->mAttributeID == id)
			{
				assert(attribute->get_type().is_derived_from(RTTI_OF(VertexAttribute<T>)));
				return static_cast<VertexAttribute<T>*>(attribute.get());
			}
		}
		return nullptr;
	}

	template<typename T>
	nap::VertexAttribute<T>& nap::MeshInstance::getAttribute(const std::string& id)
	{
		VertexAttribute<T>* attribute = findAttribute<T>(id);
		assert(attribute != nullptr);
		return *attribute;
	}

	template<typename T>
	const VertexAttribute<T>& nap::MeshInstance::getAttribute(const std::string& id) const
	{
		const VertexAttribute<T>* attribute = findAttribute<T>(id);
		assert(attribute != nullptr);
		return *attribute;
	}

	template<typename T>
	nap::VertexAttribute<T>& nap::MeshInstance::getOrCreateAttribute(const std::string& id)
	{
		for (auto& attribute : mProperties.mAttributes)
		{
			if (attribute->mAttributeID == id)
			{
				VertexAttribute<T>* result = rtti_cast<VertexAttribute<T>>(attribute.get());
				assert(result != nullptr); // Attribute found, but has wrong type!
				return *result;
			}
		}

		std::unique_ptr<VertexAttribute<T>> new_attribute = std::make_unique<VertexAttribute<T>>();
		new_attribute->mAttributeID = id;
		VertexAttribute<T>* result = new_attribute.get();
		mProperties.mAttributes.emplace_back(std::move(new_attribute));

		return *result;
	}

	template<typename T>
	const VertexAttribute<T>* nap::Mesh::FindAttribute(const std::string& id) const
	{
		for (auto& attribute : mProperties.mAttributes)
			if (attribute->mAttributeID == id)
				return static_cast<VertexAttribute<T>*>(attribute.get());

		return nullptr;
	}


	template<typename T>
	const VertexAttribute<T>& nap::Mesh::GetAttribute(const std::string& id) const
	{
		const VertexAttribute<T>* attribute = FindAttribute<T>(id);
		assert(attribute != nullptr);
		return *attribute;
	}
}
