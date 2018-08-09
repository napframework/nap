#pragma once

// External Includes
#include <utility/datetimeutils.h>
#include <color.h>

namespace nap
{
	// Forward Declares
	class AtmosApp;

	class AtmosGui final
	{
	public:
		// Constructor
		AtmosGui(AtmosApp& app);
		virtual ~AtmosGui()					{ }

		/**
		 * Initialize the various gui components
		 */
		void init();

		/**
		 *	Update all the various gui components
		 */
		void update();

		/**
		 *	Render the gui
		 */
		void render();

		/**
		 *	Toggles GUI visibility
		 */
		void toggleVisibility();

	private:
		AtmosApp&			mApp;				///< The actual atmos appliation we build the gui for
		bool				mHide = false;
		utility::DateTime	mDateTime;
		RGBColor8			mTextColor = { 0xC8, 0x69, 0x69 };

		/**
		 * Shows the controls menu
		 */
		void showControlWindow();

		/**
		 * Shows the information window
		 */
		void showInfoWindow();
	};
}