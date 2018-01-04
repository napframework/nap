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
	 * By default the plane has 1 row and 1 column
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
		glm::vec2	mSize = { 1.0, 1.0 };								///< Property: 'Size' the size of the plane in units
		glm::vec2	mPosition =	{ 0.0,0.0 };							///< Property: 'Position' where the plane is positioned in object space
		int			mRows = 1;											///< Property: 'Rows' number of rows
		int			mColumns = 1;										///< Property: 'Columns' number of columns

	protected:
		/**
		 * Creates and prepares the mesh but doesn't initialize it
		 * Call this in derived classes that want to work with a grid before initialization
		 * @param error contains the error code if setup fails
		 * @return if setup succeeded
		 */
		bool setup(utility::ErrorState& error);

	private:
		std::unique_ptr<MeshInstance> mMeshInstance;
		math::Rect mRect;

		/**
		 * Constructs the plane with x amount of columns and rows
		 * @param planeRect the rectangle used to construct the mesh
		 * @param mesh the mesh that needs to be constructed
		 */
		void constructPlane(const math::Rect& planeRect, nap::MeshInstance& mesh);
	};
}
