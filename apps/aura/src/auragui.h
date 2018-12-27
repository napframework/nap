#pragma once

// External Includes
#include <color.h>
#include <nap/resourceptr.h>
#include <imagefromfile.h>

namespace nap
{
	// Forward Declares
	class AuraApp;

	/**
	 * Gui associated with Aura Application
	 * Handles all gui update and draw related functionality
	 */
	class AuraGui
	{
	public:
		// Constructor
		AuraGui(AuraApp& app);

		// Initialize all gui elements
		void init();

		/**
		 * Update gui components
		 * @param deltaTime time in between calls in seconds
		 */
		void update(double deltaTime);

		/**
		 * Draw this gui to screen
		 */
		void draw();

	private:
		AuraApp&	mApp;
		bool		showInfo = false;											//< If the GUI should be shown
		RGBAColor8	mTextHighlightColor = { 0xC8, 0x69, 0x69, 0xFF };	//< GUI text highlight color
		
		// Gathered resources
		ResourcePtr<ImageFromFile> mColorLookupImage = nullptr;
		float mLookupSize = 1.0f;

		void showInfoWindow();											//< Shows the info window to the user
	};
}