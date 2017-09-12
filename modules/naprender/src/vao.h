#pragma once

#include "material.h"
#include "mesh.h"

#include <type_traits>

namespace opengl
{
	class VertexArrayObject;
}

namespace nap
{
	class RenderService;

	/**
	 * A vertex array object is identified by the combination of Material and MeshResource, as it
	 * binds those two objects together.
	 */
	struct VAOKey final
	{
		VAOKey() = default;
		VAOKey(const VAOKey& rhs) = default;
		VAOKey(const Material& material, const MeshInstance& meshResource);
		
		VAOKey& operator=(const VAOKey& rhs) = default;

		/**
		* Equality operator, for use in maps
		*/
		bool operator==(const VAOKey& rhs) const	{ return mMaterial == rhs.mMaterial && mMeshResource == rhs.mMeshResource; }

		const Material*			mMaterial = nullptr;
		const MeshInstance*		mMeshResource = nullptr;
	};


	/**
	 * Handle to an OpenGL VertexArrayObject object, as it is acquired from the RenderService. This object does not own
	 * the opengl VAO, it is still owned by the RenderService. On destruction of this handle, the RenderService is 
	 * automatically notified of the removal. When no more objects are referencing this VAO, it will be queued for destruction 
	 * in the RenderService.
	 */
	class VAOHandle final
	{
	public:
		VAOHandle() = default;
		VAOHandle(const VAOHandle& other);
		~VAOHandle();

		VAOHandle& operator=(const VAOHandle& rhs);

		VAOHandle(VAOHandle&& rhs);
		VAOHandle& operator=(VAOHandle&& rhs);
        
		bool isValid() const { return mObject != nullptr; }
		opengl::VertexArrayObject& get() { assert(isValid()); return *mObject; }

	private:
		friend class RenderService;

		/**
		* ctor, made private so that only RenderService can create it (through create)
		*/
		VAOHandle(RenderService& renderService, const VAOKey& key, opengl::VertexArrayObject* object);


	private:
		RenderService* mRenderService = nullptr;			///< Back pointer to RenderService, for removal on destruction
		VAOKey mKey;										///< The key for the VAO we have a handle to
		opengl::VertexArrayObject* mObject = nullptr;		///< The actual opengl object that can be used to bind and  unbind before drawing
	};
}

/**
 * hashing implementation for VAOKey
 */
namespace std
{
	template <>
	struct hash<nap::VAOKey>
	{
		std::size_t operator()(const nap::VAOKey& key) const
		{
			std::size_t value1 = std::hash<nap::Material*>{}((nap::Material*)key.mMaterial);
			std::size_t value2 = std::hash<nap::MeshInstance*>{}((nap::MeshInstance*)key.mMeshResource);
			return value1 ^ value2;
		}
	};
}

