#pragma once

// External Includes
#include <nap/datetime.h>
#include <color.h>
#include <nglutils.h>
#include <imagefromfile.h>
#include <nap/resourceptr.h>
#include <parameter.h>
#include <enumparameters.h>
#include <parameternumeric.h>
#include <parametersimple.h>
#include <parametercolor.h>

namespace nap
{
	// Forward Declares
	class AtmosApp;
	class ParameterGUI;
	class ParameterService;

	class AtmosGui final
	{
	public:
		// Constructor
		AtmosGui(AtmosApp& app);
		~AtmosGui();

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

		const bool getVisibility() const { return !mHide; }

		/**
		 *	@return the background color
		 */
		glm::vec4 getBackgroundColor() const			{ return glm::vec4(mBackgroundColor->mValue.getRed(), mBackgroundColor->mValue.getGreen(), mBackgroundColor->mValue.getBlue(), 1.0f); }

		/**
		 * @return the current draw mode
		 */
		opengl::EPolygonMode getRenderMode() const		{ return mRenderMode->mValue; }

		/**
		 * @return if the path should be rendered
		 */
		bool renderPath() const							{ return mRenderPath->mValue; }

	private:
		AtmosApp&							mApp;				///< The actual atmos application we build the gui for
		ParameterService&					mParameterService;
		std::unique_ptr<ParameterGUI>		mParameterGUI;
		bool								mHide = false;
		DateTime							mDateTime;
		RGBColor8							mTextColor = { 0xC8, 0x69, 0x69 };
		float								mTexPreviewDisplaySize = 1.0f;
		float								mWraPreviewDisplaySize = 1.0f;
		float								mVidPreviewDisplaySize = 1.0f;
		ResourcePtr<ParameterFloat>			mRotateSpeed;
		ResourcePtr<ParameterControlMethod>	mCameraControlMethod;
		ResourcePtr<ParameterBool>			mLinkFogToBackground;
		ResourcePtr<ParameterRGBColorFloat>	mBackgroundColor;
		ResourcePtr<ParameterRGBColorFloat>	mFogColor;
		ResourcePtr<ParameterBool>			mUseTransparency;
		ResourcePtr<ParameterBool>			mRenderPath;
		ResourcePtr<ParameterPolygonMode>	mRenderMode;
		ResourcePtr<ImageFromFile>			mLedOn = nullptr;				//< Signals established connection
		ResourcePtr<ImageFromFile>			mLedOff = nullptr;

		/**
		 * Shows the information window
		 */
		void showInfoWindow();

		/**
		 * Updates the fog color	
		 */
		void UpdateFogColor();
	};
}