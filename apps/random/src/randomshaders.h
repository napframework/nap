#pragma once

#include <uniforms.h>

namespace nap
{
	// Forward Declares
	class RandomApp;

	/**
	* Handles all shader related functionality for the random app
	*/
	class RandomShaders final
	{
	public:
		/**
		* Constructor, needs the random app to function
		*/
		RandomShaders(RandomApp& app);

		/**
		*	Update shader uniforms
		*/
		void update(double deltaTime);
		void updateOrbit();
		void updateCloudsInverted();

		/**
		*	Exposed properties for GUI
		*/
		glm::vec3*	pCloudsOffset = NULL;
		float*		pCloudsRotation = NULL;
		float*		pCloudsContrast = NULL;
		float*		pCloudsScale = NULL;
		float*		pCloudsInverted = NULL;
		glm::vec3*	pSunOrbitCenter = NULL;
		float*		pSunOrbitAngle = NULL;
		float*		pSunOrbitRadius = NULL;
		float*		pSunOuterSize = NULL;
		float*		pSunInnerSize = NULL;
		float*		pSunStretch = NULL;
		float		mNoiseSpeed = 0.05f;
		float		mWindSpeed = 0.1f;
		bool		mCloudsInverted = false;

	private:
		// The app used to extract information from
		RandomApp&	mApp;
	};
}
