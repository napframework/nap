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
		void updateCameraLocation();
		void startLightingModeTransition(nap::Texture2D& oldTexture, nap::Texture2D& newTexture);
		void updateLightingModeTransition(double deltaTime);

	private:
		// The app used to extract information from
		RandomApp&	mApp;

		// Properties for controlling the lighting mode transition
		bool		mLightingModeTransitionActive = false;
		float		mLightingModeTransitionVelocity = 0.0f;
	};
}
