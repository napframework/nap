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
		void showContentControls();
		void showOutputControls();
		void showSunControls();
		void showVideoControls();
		void showStaticControls();
		void showPartyControls();

		// Layout sizes
		const float guiWindowWidth = 400.0f;
		const float guiWindowPadding = 7.0f;

		// The app used to extract information from
		RandomApp&	mApp;
	};
}
