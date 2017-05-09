#pragma once

#include <nap/resource.h>
#include "nvertexarrayobject.h"
#include "ndrawutils.h"

namespace nap
{
	class Material;
	class MeshResource;

	class ModelResource : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		// Default constructor
		ModelResource() = default;

		/**
 		 * Loads model from file.
 		 */
		virtual bool init(ErrorState& errorState) override;

		/**
 		 * Performs commit or rollback of changes made in init.
 		 */
		virtual void finish(Resource::EFinishMode mode) override;

		/**
		 * @return the mesh display name
		 */
		virtual const std::string getDisplayName() const override;
		
		/**
		 * Draws the vertex data associated with this mesh object to the currently active context
		 * Calls bind before drawing.
		 */
		void draw();

		/**
		* Sets the mode used when drawing this object to a render target
		* @param mode the new draw mode
		*/
		void setDrawMode(opengl::DrawMode mode)								{ mVAO.setDrawMode(mode); }

		/**
		* Returns the current draw mode
		* @param the current draw mode
		*/
		opengl::DrawMode getDrawMode() const								{ return mVAO.getDrawMode(); }

	public:
		Material*		mMaterialResource = nullptr;
		MeshResource*	mMeshResource = nullptr;

	private:
		opengl::VertexArrayObject	mVAO;

	};


} // nap

