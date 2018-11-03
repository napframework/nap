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
		void updateSunGlareOrbit();
		void updateSunCloudsInverted();

		/**
		*	Exposed properties for GUI
		*/
		glm::vec3*	pSunCloudsOffset = NULL;
		float*		pSunCloudsRotation = NULL;
		float*		pSunCloudsContrast = NULL;
		float*		pSunCloudsScale = NULL;
		float*		pSunCloudsInverted = NULL;

		glm::vec3*	pSunGlareOrbitCenter = NULL;
		float*		pSunGlareOrbitAngle = NULL;
		float*		pSunGlareOrbitRadius = NULL;
		float*		pSunGlareOuterSize = NULL;
		float*		pSunGlareInnerSize = NULL;
		float*		pSunGlareStretch = NULL;

		float		mSunCloudsNoiseSpeed = 0.05f;
		const float	mSunCloudsNoiseSpeedMax = 0.25f;
		float		mSunCloudsWindSpeed = 0.1f;
		const float	mSunCloudsWindSpeedMax = 0.5f;
		const float mSunCloudsScaleMin = 0.1f;
		const float mSunCloudsScaleMax = 2.0f;
		bool		mSunCloudsInverted = false;

		const float	mSunGlareSizeMin = 0.05f;
		const float	mSunGlareSizeMax = 0.25f;		
		const float	mSunGlareStretchMin = 1.0f;
		const float	mSunGlareStretchMax = 10.0f;

	private:
		// The app used to extract information from
		RandomApp&	mApp;
	};
}
