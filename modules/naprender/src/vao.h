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
		/**
		 * ctor
		 */
		VAOKey(const Material& material, const Mesh& meshResource);

		/**
		* Equality operator, for use in maps
		*/
		bool operator==(const VAOKey& rhs) const	{ return &mMaterial == &rhs.mMaterial && &mMeshResource == &rhs.mMeshResource; }

		const Material&			mMaterial;
		const Mesh&		mMeshResource;
	};


	/**
	 * Handle to an OpenGL VertexArrayObject object, as it is acquired from the RenderService. This object does not own
	 * the opengl VAO, it is still owned by the RenderService. On destruction of this handle, the RenderService is 
	 * automatically notified of the removal. When no more objects are referencing this VAO, it will be queued for destruction 
	 * in the RenderService.
	 */
	class VAOHandle final
	{
    private:
        RenderService& mRenderService;		///< Back pointer to RenderService, for removal on destruction
        
	public:
		opengl::VertexArrayObject* mObject = nullptr;			///< The actual opengl object that can be used to bind and  unbind before drawing
        ~VAOHandle();
        
	private:
		friend class RenderService;

		/**
		* Helper to create a handle.
		*/
		static std::unique_ptr<VAOHandle> create(RenderService& renderService, opengl::VertexArrayObject* object);

		/**
		* ctor, made private so that only RenderService can create it (through create)
		*/
		VAOHandle(RenderService& renderService, opengl::VertexArrayObject* object);

		VAOHandle(const VAOHandle& rhs)             = delete;
		VAOHandle& operator=(const VAOHandle& rhs)  = delete;
 		VAOHandle(VAOHandle&& rhs)                  = delete;
		VAOHandle& operator=(VAOHandle&& ths)       = delete;
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
			std::size_t value1 = std::hash<nap::Material*>{}((nap::Material*)&key.mMaterial);
			std::size_t value2 = std::hash<nap::Mesh*>{}((nap::Mesh*)&key.mMeshResource);
			return value1 ^ value2;
		}
	};
}

