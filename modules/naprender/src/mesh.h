#pragma once

#include <nmesh.h>
#include <memory>
#include <utility/dllexport.h>
#include <rtti/rttiobject.h>
#include <nap/objectptr.h>

namespace nap
{
	class MeshAttribute : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:		

		std::string			mAttributeID;
	};

	template<typename ELEMENTTYPE>
	class TypedMeshAttribute : public MeshAttribute
	{
		RTTI_ENABLE(MeshAttribute)
	public:
		const std::vector<ELEMENTTYPE>& getData() const { return mData;  }

		void Reserve(size_t numElements)
		{
			mData.reserve(numElements);
		}

		void Add(const ELEMENTTYPE& element)
		{
			mData.push_back(element);
		}

		std::vector<ELEMENTTYPE>	mData;
	};

	class NAPAPI Mesh : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
		using RTTIAttributeList = std::vector<ObjectPtr<MeshAttribute>>;

		// Default constructor
		Mesh() = default;

		~Mesh();

		/**
 		 * Load the mesh
 		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the opengl mesh that can be drawn to screen or buffer
		 */
		opengl::Mesh& getMesh() const;

		template<class T>
		TypedMeshAttribute<T>& GetOrCreateAttribute(const std::string& id)
		{
			for (auto& attribute : mOwnedAttributes)
				if (attribute->mAttributeID == id)
					return static_cast<TypedMeshAttribute<T>&>(*attribute);

			std::unique_ptr<TypedMeshAttribute<T>> new_attribute = std::make_unique<TypedMeshAttribute<T>>();
			new_attribute->mAttributeID = id;

			assert(!mID.empty());
			new_attribute->mID = mID + "_" + id;

			mRTTIAttributes.push_back(ObjectPtr<MeshAttribute>(new_attribute.get()));
			mOwnedAttributes.emplace_back(std::move(new_attribute));

			return static_cast<TypedMeshAttribute<T>&>(*mOwnedAttributes[mOwnedAttributes.size() - 1]);
		}

		void ReserveIndices(size_t numIndices)
		{
			mIndices.reserve(numIndices);
		}

		void AddIndex(int index)
		{
			mIndices.push_back(index);
		}

		void update();

	protected:
		void initMesh(const RTTIAttributeList& rttiAttributes);
		
	public:
		using OwnedAttributeList = std::vector<std::unique_ptr<MeshAttribute>>;
		
		using IndexList = std::vector<int>;

		int								mNumVertices;
		opengl::EDrawMode				mDrawMode;
		OwnedAttributeList				mOwnedAttributes;
		RTTIAttributeList				mRTTIAttributes;
		IndexList						mIndices;

	protected:
		std::unique_ptr<opengl::Mesh>	mGPUMesh;
	};


	class NAPAPI MeshFromFile : public Mesh
	{
		RTTI_ENABLE(Mesh)
	
	public:
		/**
 		 * Loads model from file.
 		 */
		virtual bool init(utility::ErrorState& errorState) override;
		std::string				mPath;
	};

	using FloatMeshAttribute	= TypedMeshAttribute<float>;
	using IntMeshAttribute		= TypedMeshAttribute<int>;
	using ByteMeshAttribute		= TypedMeshAttribute<int8_t>;
	using DoubleMeshAttribute	= TypedMeshAttribute<double>;
	using Vec3MeshAttribute		= TypedMeshAttribute<glm::vec3>;
	using Vec2MeshAttribute		= TypedMeshAttribute<glm::vec2>;
	using Vec4MeshAttribute		= TypedMeshAttribute<glm::vec4>;
} // nap

