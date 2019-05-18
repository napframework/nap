#pragma once

// External Includes
#include <nap/datetime.h>
#include <color.h>
#include <nglutils.h>
#include <imagefromfile.h>
#include <nap/resourceptr.h>
#include <parameter.h>
#include <parameternumeric.h>
#include <parametersimple.h>
#include <parametercolor.h>
#include <renderablemeshcomponent.h>

namespace nap
{
	// Forward Declares
	class FlexblockApp;
	class ParameterGUI;
	class ParameterService;

	class FlexblockGui final
	{
	public:
		// Constructor
		FlexblockGui(FlexblockApp& app);
		~FlexblockGui();

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
		FlexblockApp&						mApp;				///< The actual atmos application we build the gui for
		ParameterService&					mParameterService;
		std::unique_ptr<ParameterGUI>		mParameterGUI;
		bool								mHide = false;
		DateTime							mDateTime;
		RGBColor8							mTextColor = { 0xC8, 0x69, 0x69 };
		float								mTexPreviewDisplaySize = 1.0f;
		float								mWraPreviewDisplaySize = 1.0f;
		float								mVidPreviewDisplaySize = 1.0f;

		RenderableMeshComponentInstance&	mBlockMeshInstance;
		Vec3VertexAttribute&				mVertexAttribute;
		/**
		 * Shows the information window
		 */
		void showInfoWindow();
	};
}