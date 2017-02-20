#pragma once

// External Includes
#include <nap/serviceablecomponent.h>

// Local Includes
#include "renderattributes.h"
#include "rendercomponent.h"

namespace nap
{
	/**
	 * Acts as a camera in the render system
	 * The camera does not carry a transform and therefore
	 * only defines a projection matrix.
	 */
	class CameraComponent : public ServiceableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(ServiceableComponent)
	public:
		// Default constructor
		CameraComponent();

		/**
		 * Controls the camera field of view, ie:
		 * the extent of the observable world
		 */
		Attribute<float> fieldOfView =			{ this, "fieldOfView", 50.0f };
		
		/**
		 * Camera aspect ratio, most likely
		 * needs to match render target aspect ratio
		 */
		Attribute<float> aspectRatio =			{ this, "aspectRatio", 1.0f };
		
		/**
		 * Camera clipping planes, ie:
		 * what is considered to be in bounds of observable
		 * world
		 */
		Attribute<glm::vec2> clippingPlanes =	{ this, "clippingPlanes", {1.0f, 1000.0f} };

		/**
		* Convenience method to specicy lens aspect ratio, defined as width / height
		* @param width, arbitrary width, most often the resolution of the canvas
		* @param height, arbitrary height, most often the resolution of the canvas
		*/
		void	setAspectRatio(float width, float height);

		/**
		* @return camera projection matrix
		* Use this matrix to transform a 3d scene in to a 2d projection
		*/
		const glm::mat4& getProjectionMatrix() const;

		/**
		 * Sets this camera to be dirty, ie: 
		 * next time the matrix is queried it is recomputed
		 */
		void setDirty()							{ mDirty = true; }

		/**
		 * Sets the object to look at
		 */
		ObjectLinkAttribute lookAt				{ this, "lookAt", RTTI_OF(RenderableComponent) };

	private:
		// The composed projection matrix
		mutable glm::mat4x4 mProjectionMatrix;

		// If the projection matrix needs to be recalculated
		mutable bool mDirty = true;

		// Slots
		void onAttribValueChanged(nap::AttributeBase& v)		{ setDirty(); }
		NSLOT(cameraValueChanged, nap::AttributeBase&, onAttribValueChanged)
	};
}

RTTI_DECLARE(nap::CameraComponent)
