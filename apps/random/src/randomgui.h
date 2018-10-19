#pragma once

#include <color.h>

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

		/**
		 *	Shows the information window
		 */
		void showInfoWindow();

		/**
		*	Set the position and radius of the orbit circle
		*/
		void setOrbitTransform(float *x, float *z, float *radius);

		// Initialized Variables
		RGBAColor8	mTextHighlightColor = { 0xC8, 0x69, 0x69, 0xFF };
		float		mTextureDisplaySize = 0.5f;
		float		mNoiseSpeed = 0.1f;
		float		mWindSpeed = 0.5f;
		bool		mCloudsInverted = false;

		// The app used to extract information from
		RandomApp&	mApp;

		// Window visibility toggles
		bool		mShowControls = true;		//< If gui controls are shown
		bool		mShowInfo = true;			//< If gui info is shown

		// Define constant values
		static const float uvScale;
		static const float mainMenuHeight;
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
