#pragma once

// External Includes
#include <nap/datetime.h>
#include <color.h>
#include <nglutils.h>
#include "parameter.h"

namespace nap
{
	// Forward Declares
	class AtmosApp;
	class ParameterGUI;

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
		const glm::vec4& getBackgroundColor() const			{ return mBackgroundColor; }

		/**
		 * @return the current draw mode
		 */
		opengl::EPolygonMode getRenderMode() const			{ return mRenderMode; }

	private:
		AtmosApp&							mApp;				///< The actual atmos appliation we build the gui for
		ParameterService&					mParameterService;
		std::unique_ptr<ParameterGUI>		mParameterGUI;
		bool								mHide = false;
		DateTime							mDateTime;
		RGBColor8							mTextColor = { 0xC8, 0x69, 0x69 };
		glm::vec4							mBackgroundColor;
		bool								mTransparent = false;
		opengl::EPolygonMode				mRenderMode = opengl::EPolygonMode::Fill;
		float								mTexPreviewDisplaySize = 1.0f;
		float								mWraPreviewDisplaySize = 1.0f;
		float								mVidPreviewDisplaySize = 1.0f;
		ResourcePtr<ParameterFloat>			mCameraMovSpeed;
		ResourcePtr<ParameterFloat>			mCameraRotSpeed;
		ResourcePtr<ParameterFloat>			mRotateSpeed;
		float								mCamMaxRotSpeed;
		float								mCamMaxMovSpeed;
		bool								mLinkFogToBackground = true;
		bool								mBackgroundColorDirty = true;

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