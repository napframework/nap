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

		// Initialized Variables
		RGBAColor8	mTextHighlightColor = { 0xC8, 0x69, 0x69, 0xFF };
		float		mTextureDisplaySize = 0.5f;
		float		mNoiseSpeed = 0.1f;
		float		mWindSpeed = 0.1f;
		bool		mCloudsInverted = false;

		// The app used to extract information from
		RandomApp&	mApp;

		// Window visibility toggles
		bool		mShowControls = false;		//< If gui controls are shown
		bool		mShowInfo = false;			//< If gui info is shown
	};
}