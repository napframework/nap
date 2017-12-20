#pragma once

// Local Includes
#include "vertexattribute.h"

// External includes
#include <ngpumesh.h>
#include <memory>
#include <utility/dllexport.h>
#include <rtti/rttiobject.h>
#include <nap/objectptr.h>
#include <nap/configure.h>

namespace nap
{
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
		using IndexList = std::vector<unsigned int>;

		int								mNumVertices;
		opengl::EDrawMode				mDrawMode;
		VertexAttributeList				mAttributes;
		IndexList						mIndices;
	};

	// ObjectPtr based mesh properties, used in serializable Mesh format (json/binary)
	using RTTIMeshProperties = MeshProperties<ObjectPtr<BaseVertexAttribute>>;


	/**
	 * Represents a runtime version of a mesh. MeshInstance holds CPU data and can convert this data to 
	 * an opengl::GPUMesh. 
	 * A MeshInstance can be created in two ways:
	 *		1) Manually allocate a MeshInstance object, add attributes to it, and call the regular init() on it. 
	 *		   This is useful for creation of procedural meshes.
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
		using VertexAttributeID = std::string;

		/**
		 * Known vertex attribute IDs in the system, used for loading/creating meshes with well-known attributes.
		 */
		struct VertexAttributeIDs
		{
			static const NAPAPI VertexAttributeID GetPositionName();	//< Default position vertex attribute name
			static const NAPAPI VertexAttributeID getNormalName();		//< Default normal vertex attribute name
			static const NAPAPI VertexAttributeID getTangentName();		//< Default tangent vertex attribute name
			static const NAPAPI VertexAttributeID getBitangentName();	//< Default bi-tangent vertex attribute name

			/**
			 * Returns the name of the vertex uv attribute based on the queried uv channel
			 * @param uvChannel: the uv channel index to query
			 * @return the name of the vertex attribute
			 */
			static const NAPAPI VertexAttributeID GetUVName(int uvChannel);

			/**
			 *	Returns the name of the vertex color attribute based on the queried uv channel
			 * @param colorChannel: the color channel index to query
			 * @return the name of the color vertex attribute
			 */
			static const NAPAPI VertexAttributeID GetColorName(int colorChannel);
		};

		// Default constructor
		MeshInstance() = default;

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
		 * @param errorState Contains error information if an error occurred.
		 * @return True if succeeded, false on error.
		 */
		void copyMeshProperties(RTTIMeshProperties& meshProperties);

		/**
		 * @return the opengl mesh that can be drawn to screen or buffer
		 */
		opengl::GPUMesh& getGPUMesh() const;

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
		 * Clears the list of indices. 
		 * Call either before init() or call update() to reflect the changes in the GPU buffer.
		 */
		void clearIndices()														{ mProperties.mIndices.clear(); }

		/**
	 	 * Reserves CPU memory for index list. GPU memory is reserved after update() is called.
		 * @param numIndices Amount of indices to reserve.
		 */
		void reserveVertices(size_t numVertices);

		/**
		 * Reserves CPU memory for index list. GPU memory is reserved after update() is called.
		 * @param numIndices Amount of indices to reserve.
		 */
		void reserveIndices(size_t numIndices);

		/**
		 * Adds a single index to the index CPU buffer. Use setIndices to add an entire list of indices.
		 * Call either before init() or call update() to reflect the changes in the GPU buffer.
		 * @param index Index to add.
		 */
		void addIndex(int index)												{ mProperties.mIndices.push_back(index); }

		/**
 		 * Adds a number of indices to the existing indices in the index CPU buffer. Use setIndices to replace
		 * the current indices with a new set of indices.
		 * Call either before init() or call update() to reflect the changes in the GPU buffer.
		 * @param indices List of indices to update.
		 * @param numIndices Number of indices in the list.
		 */
		void addIndices(uint32_t* indices, int numIndices);

		/**
		 * Adds a list of indices to the index CPU buffer.
		 * Call either before init() or call update() to reflect the changes in the GPU buffer.
		 * @param indices: array of indices to add.
		 * @param numIndices: number of indices in @indices.
		 */
		void setIndices(uint32_t* indices, int numIndices);

		/**
		 * @return if the mesh has indices associated with it
		 */
		bool hasIndices() const { return !(mProperties.mIndices.empty()); }

		/**
		 * @return the indices associated with this mesh. This array is empty
		 * if this mesh has no indices
		 */
		const std::vector<uint>& getIndices() const { return mProperties.mIndices; }

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
		 * Sets Draw mode for this mesh
		 * @param drawMode: OpenGL draw mode.
		 */
		void setDrawMode(opengl::EDrawMode drawMode)							{ mProperties.mDrawMode = drawMode; }

		/**
		 * @return Draw mode for this mesh.
		 */
		opengl::EDrawMode getDrawMode() const									{ return mProperties.mDrawMode; }

		/**
		 * Uses the CPU mesh data to update the GPU mesh. Note that update() is called during init(),
		 * so this is only required if CPU data is modified after init().
		 * If there is a mismatch between vertex buffer, an error will be returned.
		 * @param errorState Contains error information if an error occurred.
		 * @return True if succeeded, false on error.		 
		 */
		bool update(utility::ErrorState& errorState);

	protected:
		bool initGPUData(utility::ErrorState& errorState);

	private:
		MeshProperties<std::unique_ptr<BaseVertexAttribute>>	mProperties;		///< CPU mesh data
		std::unique_ptr<opengl::GPUMesh>						mGPUMesh;			///< GPU mesh
	};


	/**
	 * Base class for each mesh resource. Every derived mesh should provide a MeshInstance class.
	 */
	class IMesh : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
		virtual MeshInstance& getMeshInstance() = 0;
		virtual const MeshInstance& getMeshInstance() const = 0;
	};


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
		 * Initialized the mesh instance.
		 * @param errorState Contains error information if an error occurred.
		 * @return True if succeeded, false on error.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return MeshInstance as created during init().
		 */
		virtual MeshInstance& getMeshInstance()	override		{ return mMeshInstance; }

		/**
		 * @return MeshInstance as created during init().
		 */
		virtual const MeshInstance& getMeshInstance() const	override { return mMeshInstance; }

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
		const VertexAttribute<T>& GetAttribute(const std::string& id) const
		{
			const VertexAttribute<T>* attribute = FindAttribute<T>(id);
			assert(attribute != nullptr);
			return *attribute;
		}

		RTTIMeshProperties	mProperties;		///< RTTI mesh CPU data

	private:
		MeshInstance		mMeshInstance;		///< Runtime mesh instance
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
			if (attribute->mAttributeID == id)
			{
				assert(attribute->get_type().is_derived_from(RTTI_OF(VertexAttribute<T>)));
				return static_cast<VertexAttribute<T>*>(attribute.get());
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

} // nap

