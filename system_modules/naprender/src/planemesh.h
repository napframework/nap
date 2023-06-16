/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "mesh.h"

// External includes
#include <rect.h>
#include <color.h>
#include <nap/numeric.h>

namespace nap
{
	// Forward Declares
	class Core;
	class RenderService;

	/**
	 * Predefined rectangular mesh with a specific size centered at the origin on the xy axis. 
	 * When there is no size given the mesh is a uniform 1m2. The UV coordinates are always 0-1.
	 * By default the plane has 1 row and 1 column.
	 */
	class NAPAPI PlaneMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		PlaneMesh(Core& core);

		/**
		 * Sets up, initializes and uploads the plane to the GPU based on the provided parameters.
		 * @param errorState contains the error message if the mesh could not be created.
		 * @return if the mesh was successfully created and initialized.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		* Creates and prepares the mesh instance but doesn't initialize it.
		* Call this when you want to prepare a grid without creating the GPU representation.
		* You have to manually call init() on the mesh instance afterwards.
		* @param error contains the error code if setup fails
		* @return if setup succeeded
		*/
		bool setup(utility::ErrorState& error);

		/**
		 *	@return the mesh used for rendering
		 */
		virtual MeshInstance& getMeshInstance() override					{ return *mMeshInstance; }

		/**
		 *	@return the mesh used for rendering
		 */
		virtual const MeshInstance& getMeshInstance() const override		{ return *mMeshInstance; }

		/**
		 *	@return the plane as a rectangle
		 */
		const math::Rect& getRect()											{ return mRect; }

		// property: the size of the plane
		glm::vec2		mSize			= { 1.0, 1.0 };						///< Property: 'Size' the size of the plane in units
		glm::vec2		mPosition		= { 0.0,0.0 };						///< Property: 'Position' where the plane is positioned in object space
		RGBAColorFloat	mColor			= { 1.0f, 1.0f, 1.0f, 1.0f };		///< Property: 'Color' color of the plane
		uint			mRows			= 1;								///< Property: 'Rows' number of rows
		uint			mColumns		= 1;								///< Property: 'Columns' number of columns
		EMemoryUsage	mUsage			= EMemoryUsage::Static;			///< Property: 'Usage' If the plane is uploaded once or frequently updated.
		ECullMode		mCullMode		= ECullMode::None;					///< Property: 'CullMode' Plane cull mode, defaults to no culling
		EPolygonMode	mPolygonMode	= EPolygonMode::Fill;				///< Property: 'PolygonMode' Polygon rasterization mode (fill, line, points) 

	private:
		RenderService* mRenderService;
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
