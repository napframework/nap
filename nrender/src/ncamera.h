#pragma once

#include <glm/glm.hpp>

namespace opengl
{
	/**
	 * Projects a 3d scene onto a 'canvas', ie: flattening it
	 * The camera converts points from eye space in to clip space.
	 * First, the camera transforms all vertex data from the eye coordinates to the clip coordinates. 
	 * Then, these clip coordinates are also transformed to the normalized device coordinates (NDC). 
	 *
	 * Note that the camera does not hold any transform information
	 * The camera can best be described as a camera without a person that holds it and moves it around
	 * for more information on projection matrices: http://www.songho.ca/opengl/gl_projectionmatrix.html
	 */
	class Camera
	{
	public:
		// Default Constructor
		Camera();

		// Default Destructor
		virtual ~Camera() = default;

		/**
		 * Sets the camera field of view
		 * Field of view defines the extent of the observable world
		 * Higher values will increase this extent
		 */
		void	setFieldOfView(float value);
		
		/**
		 * @return camera field of view
		 */
		float	getFieldOfView() const							{ return mFov; }

		/**
		 * Sets the camera lens aspect ratio, which is defined as width / height
		 */
		void	setAspectRatio(float value);
		
		/**
		 * Convenience method to specicy lens aspect ratio, defined as width / height
		 * @param width, arbitrary width, most often the resolution of the canvas
		 * @param height, arbitrary height, most often the resolution of the canvas
		 */
		void	setAspectRatio(float width, float height);
		
		/**
		 * @return camera lens aspect ratio
		 */
		float	getAspectRatio() const							{ return mAspectRatio; }

		/**
		 * Sets camera near and fat clipping planes
		 * @param near camera near clipping plane
		 * @param far camera far clipping plane
		 */
		void	setClippingPlanes(float near, float far);
		
		/**
		 * Sets camera near clipping plane
		 * @param value the new near clipping plane, points below this threshold won't be included
		 */
		void	setNearClippingPlane(float value);

		/**
		 * Sets camera far clipping plane
		 * @param value the new far clipping plane, points over this threshold won't be included
		 */
		void	setFarClippingPlane(float value);

		/**
		 * @return camera near clipping plane, points below this threshold won't be included
		 */
		float	getNearClippingPlane() const					{ return mNearClipPlane; }
		
		/**
		 * @return camera far clipping plane, points beyond this threshold won't be included
		 */
		float	getFarClippingPlane() const						{ return mFarClipPlane; }

		/**
		 * Returns the camera clipping planes
		 * @param near output for the near clipping plane
		 * @param far output for the far clipping plane
		 */
		void	getClippingPlanes(float& near, float& far);

		/**
		 * @return camera projection matrix
		 * Use this matrix to transform a 3d scene in to a 2d projection
		 */
		const glm::mat4& getProjectionMatrix() const			{ return mProjectionMatrix; }

	private:
		// The projection matrix
		glm::mat4	mProjectionMatrix;

		// Members
		float mFov				= 50.0f;		// Field of view
		float mAspectRatio		= 1.0f;			// Aspect ratio (width / height)
		float mNearClipPlane	= 1.0f ;		// Near clip Plane
		float mFarClipPlane		= 1000.0f;		// Far clip plane

		/**
		 * Updates the projection matrix every time 
		 * any of the camera settings change;
		 */
		void updateProjectionMatrix();
	};
}	// opengl
