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
		 * Update gui components
		 * @param deltaTime time in between calls in seconds
		 */
		void update(double deltaTime);

		/**
		 *	Draws the gui to screen
		 */
		void draw();

	private:
		// Show control windows
		void showControlWindow();
		void showSunControls();
		void showVideoControls();
		void showStaticControls();

		// Layout sizes
		const float guiWindowWidth = 400.0f;
		const float guiWindowPadding = 7.0f;

		// Properties
		const char*	mModes[3] = { "Sun", "Video", "Static" };
		int			mCurrentMode = 0;
		float		mNoiseSpeed = 0.05f;
		float		mWindSpeed = 0.1f;
		bool		mCloudsInverted = false;

		// Property ranges
		const float cloudsScaleMin = 0.1f;
		const float cloudsScaleMax = 2.0f;
		const float orbitCenterRange = 1.5f;
		const float orbitRadiusMin = 0.5f;
		const float orbitRadiusMax = 1.5f;		
		const float sunSizeMin = 0.05f;
		const float sunSizeMax = 0.2f;		
		const float sunStretchMin = 1.0f;
		const float sunStretchMax = 10.0f;

		// The app used to extract information from
		RandomApp&	mApp;
	};
}
