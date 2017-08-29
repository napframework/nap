#pragma once

#include <nmesh.h>
#include <memory>
#include <utility/dllexport.h>
#include <rtti/rttiobject.h>
#include <nap/objectptr.h>

namespace nap
{
	class VertexAttribute : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:		
		virtual void* getData() = 0;
		virtual GLenum getType() const = 0;
		virtual int getNumComponents() const = 0;

		std::string			mAttributeID;
	};

	template<typename ELEMENTTYPE>
	class TypedVertexAttribute : public VertexAttribute
	{
		RTTI_ENABLE(VertexAttribute)
	public:
		const std::vector<ELEMENTTYPE>& getData() const { return mData;  }

		void reserve(size_t numElements)
		{
			mData.reserve(numElements);
		}

		void add(const ELEMENTTYPE& element)
		{
			mData.push_back(element);
		}

		void setData(const ELEMENTTYPE* elements, int numElements)
		{
			mData.resize(numElements);
			memcpy(mData.data(), elements, numElements * sizeof(ELEMENTTYPE));
		}

		virtual void* getData() override
		{
			return static_cast<void*>(mData.data());
		}

		virtual GLenum getType() const override { assert(false); return GL_FLOAT; } // Not implemented for this type

		virtual int getNumComponents() const override { assert(false); return -1; } // Not implemented for this type

		std::vector<ELEMENTTYPE>	mData;
	};

	class NAPAPI Mesh : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
		using VertexAttributeID = std::string;

		/**
		* Known vertex attribute IDs in the system, used for loading/creating meshes with well-known attributes.
		*/
		struct VertexAttributeIDs
		{
			static const VertexAttributeID PositionVertexAttr;	//< Default position vertex attribute name
			static const VertexAttributeID NormalVertexAttr;	//< Default normal vertex attribute name
			static const VertexAttributeID UVVertexAttr;		//< Default uv vertex attribute name
			static const VertexAttributeID ColorVertexAttr;		//< Default color vertex attribute name

																/**
																* Returns the name of the vertex uv attribute based on the queried uv channel
																* @param uvChannel: the uv channel index to query
																* @return the name of the vertex attribute
																*/
			static const VertexAttributeID GetUVVertexAttr(int uvChannel);

			/**
			*	Returns the name of the vertex color attribute based on the queried uv channel
			* @param colorChannel: the color channel index to query
			* @return the name of the color vertex attribute
			*/
			static const VertexAttributeID GetColorVertexAttr(int colorChannel);
		};

		using RTTIAttributeList = std::vector<ObjectPtr<VertexAttribute>>;

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
		opengl::GPUMesh& getGPUMesh() const;

		template<class T>
		TypedVertexAttribute<T>& GetOrCreateAttribute(const std::string& id)
		{
			for (auto& attribute : mOwnedAttributes)
				if (attribute->mAttributeID == id)
					return static_cast<TypedVertexAttribute<T>&>(*attribute);

			std::unique_ptr<TypedVertexAttribute<T>> new_attribute = std::make_unique<TypedVertexAttribute<T>>();
			new_attribute->mAttributeID = id;

			assert(!mID.empty());
			new_attribute->mID = mID + "_" + id;

			mRTTIAttributes.push_back(ObjectPtr<VertexAttribute>(new_attribute.get()));
			mOwnedAttributes.emplace_back(std::move(new_attribute));

			return static_cast<TypedVertexAttribute<T>&>(*mOwnedAttributes[mOwnedAttributes.size() - 1]);
		}

		void ReserveIndices(size_t numIndices)
		{
			mIndices.reserve(numIndices);
		}

		void AddIndex(int index)
		{
			mIndices.push_back(index);
		}

		void setIndices(uint32_t* indices, int numIndices)
		{
			mIndices.resize(numIndices);
			std::memcpy(mIndices.data(), indices, numIndices * sizeof(uint32_t));
		}

		void update();

	protected:
		void moveFrom(Mesh& mesh);

	public:
		using OwnedAttributeList = std::vector<std::unique_ptr<VertexAttribute>>;		
		using IndexList = std::vector<unsigned int>;

		int								mNumVertices;
		opengl::EDrawMode				mDrawMode;
		OwnedAttributeList				mOwnedAttributes;
		RTTIAttributeList				mRTTIAttributes;
		IndexList						mIndices;

	protected:
		std::unique_ptr<opengl::GPUMesh>	mGPUMesh;
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

	using FloatMeshAttribute	= TypedVertexAttribute<float>;
	using IntMeshAttribute		= TypedVertexAttribute<int>;
	using ByteMeshAttribute		= TypedVertexAttribute<int8_t>;
	using DoubleMeshAttribute	= TypedVertexAttribute<double>;
	using Vec2MeshAttribute		= TypedVertexAttribute<glm::vec2>;
	using Vec3MeshAttribute		= TypedVertexAttribute<glm::vec3>;
	using Vec4MeshAttribute		= TypedVertexAttribute<glm::vec4>;

	template<>
	GLenum FloatMeshAttribute::getType() const { return GL_FLOAT; }

	template<>
	int FloatMeshAttribute::getNumComponents() const { return 1; }

	template<>
	GLenum IntMeshAttribute::getType() const { return GL_INT; }

	template<>
	int IntMeshAttribute::getNumComponents() const { return 1; }

	template<>
	GLenum ByteMeshAttribute::getType() const { return GL_BYTE; }

	template<>
	int ByteMeshAttribute::getNumComponents() const { return 1; }

	template<>
	GLenum DoubleMeshAttribute::getType() const { return GL_DOUBLE; }

	template<>
	int DoubleMeshAttribute::getNumComponents() const { return 1; }

	template<>
	GLenum Vec2MeshAttribute::getType() const { return GL_FLOAT; }

	template<>
	int Vec2MeshAttribute::getNumComponents() const { return 2; }

	template<>
	GLenum Vec3MeshAttribute::getType() const { return GL_FLOAT; }

	template<>
	int Vec3MeshAttribute::getNumComponents() const { return 3; }

	template<>
	GLenum Vec4MeshAttribute::getType() const { return GL_FLOAT; }

	template<>
	int Vec4MeshAttribute::getNumComponents() const { return 4; }

} // nap

