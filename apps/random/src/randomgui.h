#pragma once

namespace nap
{
	// Forward Declares
	class RandomApp;

	/**
	 * Handles all GUI related events / functionality for the random app
	 */
	class RandomGui final
	{
	public:
		/**
		 * Constructor, needs the random app to function
		 */
		RandomGui(RandomApp& app);

		/**
		 *	Initialize all the gui components
		 */
		void init();

		/**
		 * Update gui components
		 * @param deltaTime time in between calls in seconds
		 */
		void update(double deltaTime);

		/**
		 *	Draws the gui to screen
		 */
		void draw();

	private:
		/**
		 *	Shows the controls window
		 */
		void showControlWindow();
		void showSunControls();
		void showVideoControls();
		void showStaticControls();

		/**
		*	Utilities to modify the orbit circle and elements
		*/
		float getOrbitAngle();
		void setOrbitPosition(float *x, float *z);
		void setOrbitPathRadius(float *radius);
		void setOrbitSunPosition(float *angle, float *radius);
		void setOrbitStartEndPosition(float *radius);

		// Initialized Variables
		const char*	mModes[3] = { "Sun", "Video", "Static" };
		int			mCurrentMode = 0;
		float		mNoiseSpeed = 0.05f;
		float		mWindSpeed = 0.1f;
		bool		mCloudsInverted = false;
		float		mOrbitStartEnd[2] = { 40.0f, 115.0f };
		float		mOrbitProgress = 0.0f;

		// The app used to extract information from
		RandomApp&	mApp;

		// Define constant values
		static const glm::vec2 uvOffset;
		static const float uvScale;
		static const float guiWindowWidth;
		static const float guiWindowPadding;
		static const float cloudsScaleMin;
		static const float cloudsScaleMax;
		static const float orbitCenterRange;
		static const float orbitRadiusMin;
		static const float orbitRadiusMax;
		static const float sunSizeMin;
		static const float sunSizeMax;
		static const float sunStretchMin;
		static const float sunStretchMax;
	};
}
