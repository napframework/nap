#pragma once

// External Includes
#include <nap/datetime.h>
#include <color.h>
#include <nglutils.h>
#include "parameter.h"
#include "controlselectcomponent.h"
#include "parameterenum.h"
#include "parameternumeric.h"
#include "parametersimple.h"
#include "parametercolor.h"

namespace nap
{
	// Forward Declares
	class AtmosApp;
	class ParameterGUI;
	class ParameterService;

	using ParameterControlMethod = ParameterEnum<EControlMethod>;
	using ParameterPolygonMode = ParameterEnum<opengl::EPolygonMode>;

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

		/**
		 *	@return the background color
		 */
		glm::vec4 getBackgroundColor() const			{ return glm::vec4(mBackgroundColor->mValue.getRed(), mBackgroundColor->mValue.getGreen(), mBackgroundColor->mValue.getBlue(), 1.0f); }

		/**
		 * @return the current draw mode
		 */
		opengl::EPolygonMode getRenderMode() const			{ return mRenderMode->mValue; }

	private:
		void UpdateFogColor();

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
		ResourcePtr<ParameterFloat>			mCameraMovSpeed;
		ResourcePtr<ParameterFloat>			mCameraRotSpeed;
		ResourcePtr<ParameterFloat>			mRotateSpeed;
		ResourcePtr<ParameterFloat>			mCameraFOV;
		ResourcePtr<ParameterControlMethod>	mCameraControlMethod;
		float								mCamMaxRotSpeed;
		float								mCamMaxMovSpeed;
		ResourcePtr<ParameterBool>			mLinkFogToBackground;
		ResourcePtr<ParameterRGBColorFloat>	mBackgroundColor;
		ResourcePtr<ParameterRGBColorFloat>	mFogColor;
		ResourcePtr<ParameterBool>			mUseTransparency;
		ResourcePtr<ParameterPolygonMode>	mRenderMode;

		/**
		 * Shows the information window
		 */
		void showInfoWindow();
	};
}