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

	private:
		std::unique_ptr<opengl::GPUMesh>	mGPUMesh;
	};
} // nap

