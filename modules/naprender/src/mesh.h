#pragma once

#include <ngpumesh.h>
#include <memory>
#include <utility/dllexport.h>
#include <rtti/rttiobject.h>
#include <nap/objectptr.h>
#include <glm/glm.hpp>
#include "vertexattribute.h"

namespace nap
{
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

	using RTTIMeshProperties = MeshProperties<ObjectPtr<VertexAttribute>>;

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
			static const NAPAPI VertexAttributeID GetPositionVertexAttr();	//< Default position vertex attribute name
			static const NAPAPI VertexAttributeID GetNormalVertexAttr();	//< Default normal vertex attribute name

			/**
			* Returns the name of the vertex uv attribute based on the queried uv channel
			* @param uvChannel: the uv channel index to query
			* @return the name of the vertex attribute
			*/
			static const NAPAPI VertexAttributeID GetUVVertexAttr(int uvChannel);

			/**
			*	Returns the name of the vertex color attribute based on the queried uv channel
			* @param colorChannel: the color channel index to query
			* @return the name of the color vertex attribute
			*/
			static const NAPAPI VertexAttributeID GetColorVertexAttr(int colorChannel);
		};

		// Default constructor
		MeshInstance() = default;

		virtual ~MeshInstance();

		/**
 		 * Load the mesh
 		 */
		bool init(utility::ErrorState& errorState);

		bool init(RTTIMeshProperties& meshProperties, utility::ErrorState& errorState);

		/**
		 * @return the opengl mesh that can be drawn to screen or buffer
		 */
		opengl::GPUMesh& getGPUMesh() const;

		template<typename T>
		TypedVertexAttribute<T>* FindAttribute(const std::string& id)
		{
			for (auto& attribute : mProperties.mAttributes)
				if (attribute->mAttributeID == id)
					return static_cast<TypedVertexAttribute<T>*>(attribute.get());

			return nullptr;
		}

		template<typename T>
		TypedVertexAttribute<T>& GetAttribute(const std::string& id)
		{
			TypedVertexAttribute<T>* attribute = FindAttribute<T>(id);
			assert(attribute != nullptr);
			return *attribute;
		}

		template<typename T>
		TypedVertexAttribute<T>& GetOrCreateAttribute(const std::string& id)
		{
			TypedVertexAttribute<T>* attribute = FindAttribute<T>(id);
			if (attribute == nullptr)
			{
				std::unique_ptr<TypedVertexAttribute<T>> new_attribute = std::make_unique<TypedVertexAttribute<T>>();
				new_attribute->mAttributeID = id;
				attribute = new_attribute.get();
				mProperties.mAttributes.emplace_back(std::move(new_attribute));
			}

			return *attribute;
		}

		void ReserveIndices(size_t numIndices)
		{
			mProperties.mIndices.reserve(numIndices);
		}

		void AddIndex(int index)
		{
			mProperties.mIndices.push_back(index);
		}

		void setIndices(uint32_t* indices, int numIndices)
		{
			mProperties.mIndices.resize(numIndices);
			std::memcpy(mProperties.mIndices.data(), indices, numIndices * sizeof(uint32_t));
		}

		void setNumVertices(int numVertices)
		{
			mProperties.mNumVertices = numVertices;
		}

		void setDrawMode(opengl::EDrawMode drawMode)
		{
			mProperties.mDrawMode = drawMode;
		}

		opengl::EDrawMode getDrawMode() const { return mProperties.mDrawMode; }

		int getNumVertices() const { return mProperties.mNumVertices; }

		void update();

	protected:
		void initGPUData();

	private:
		MeshProperties<std::unique_ptr<VertexAttribute>>	mProperties;
		std::unique_ptr<opengl::GPUMesh>					mGPUMesh;
	};

	class IMesh : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
		virtual MeshInstance& getMeshInstance() = 0;
		virtual const MeshInstance& getMeshInstance() const = 0;
	};

	class Mesh : public IMesh
	{
		RTTI_ENABLE(IMesh)

	public:
		virtual bool init(utility::ErrorState& errorState) override;

		virtual MeshInstance& getMeshInstance()				{ return mMeshInstance; }
		virtual const MeshInstance& getMeshInstance() const	{ return mMeshInstance; }

		template<typename T>
		const TypedVertexAttribute<T>* FindAttribute(const std::string& id) const
		{
			for (auto& attribute : mProperties.mAttributes)
				if (attribute->mAttributeID == id)
					return static_cast<TypedVertexAttribute<T>*>(attribute.get());

			return nullptr;
		}

		template<typename T>
		const TypedVertexAttribute<T>& GetAttribute(const std::string& id) const
		{
			const TypedVertexAttribute<T>* attribute = FindAttribute<T>(id);
			assert(attribute != nullptr);
			return *attribute;
		}

		RTTIMeshProperties	mProperties;

	private:
		MeshInstance	mMeshInstance;
	};

} // nap

