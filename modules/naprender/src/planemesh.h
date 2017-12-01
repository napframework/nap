#pragma once

// Local Includes
#include "mesh.h"

// External includes
#include <rect.h>

namespace nap
{
	/**
	 * Predefined rectangular mesh with a specific size centered at the origin on the xy axis. 
	 * When there is no size given the mesh is a uniform 1m2. The UV coordinates are always 0-1
	 */
	class NAPAPI PlaneMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		/**
		 * Creates and initializes the plane as a mesh
		 * @param errorState contains the error message if the mesh could not be created
		 * @return if the mesh was successfully created and initialized
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 *	@return the mesh used for rendering
		 */
		virtual MeshInstance& getMeshInstance() override				{ return *mMeshInstance; }

		/**
		 *	@return the mesh used for rendering
		 */
		virtual const MeshInstance& getMeshInstance() const override	{ return *mMeshInstance; }

		/**
		 *	@return the plane as a rectangle
		 */
		const math::Rect& getRect()										{ return mRect; }

		// property: the size of the plane
		glm::vec2 mSize = { 1.0, 1.0 };									///< Property: Size the size of the plane in units
		glm::vec2 mPosition = { 0.0,0.0 };								///< Property: Position where the plane is positioned in object space

	private:
		std::unique_ptr<MeshInstance> mMeshInstance;
		math::Rect mRect;
	};
}
