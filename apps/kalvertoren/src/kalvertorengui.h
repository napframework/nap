#pragma once

#include <color.h>
#include <glm/glm.hpp>
#include <utility/datetimeutils.h>
#include <queue>
#include <rtti/objectptr.h>
#include <imagefromfile.h>

namespace nap
{
	// Forward declares
	class KalvertorenApp;

	// GUI
	class KalvertorenGui final
	{
	public:
		/**
		 * Constructor
		 */
		KalvertorenGui(KalvertorenApp& app);

		/**
		 *	Destructor
		 */
		virtual ~KalvertorenGui()				{ }

		/**
		 *	Initializes the gui
		 */
		void init();

		/**
		 *	Updates the gui
		 */
		void update(double deltaTime);

		/**
		 *	Render the gui
		 */
		void draw();

	private:
		KalvertorenApp& mApp;

		// Resources
		rtti::ObjectPtr<ImageFromFile>						mLedOn  = nullptr;
		rtti::ObjectPtr<ImageFromFile>						mLedOff = nullptr;

		// GUI
		int													mMeshSelection = 0;
		int													mPaintMode = 2;
		int													mSelectChannel = 0;
		float												mChannelSpeed = 1.0f;
		int													mPaletteSelection = 0;
		int													mCompositionSelection = 0;
		float												mIntensity = 1.0f;
		float												mSensorInfluence = 1.0f;
		float												mLightSmoothTime = 1.0f;
		glm::vec2											mLightRange = { 0.1f, 1.0f };
		glm::vec2											mLuxRange = { 10.0f, 25000.0f };
		float												mLuxCurve = 1.0f;
		bool												mShowIndexColors = false;
		float												mDurationScale = 1.0f;
		bool												mCycleColors = false;
		float												mColorCycleTime = 1.0f;
		int													mCompositionCycleMode = 0;
		int													mSelectedWeek = 1;
		int													mColorPaletteCycleMode = 0;
		int													mDay = 0;
		utility::DateTime									mDateTime;
		RGBColor8											mTextColor = { 0xC8, 0x69, 0x69 };
		float												mDisplaySize = 0.5f;

		// Information
		std::array<float, 512>								mLuxValues;
		std::array<float, 512>								mBrightnessValues;
		float												mLuxSampleTime = 1.0f;
		int													mLuxIdx		= 0;
		int													mBrightnessIdx = 0;
		float												mLuxTime	= 0.0f;
		glm::vec2											mLuxDisplayBounds = { 0, 2000.0f };

		/**
		 *	Shows the controls menu
		 */
		void showControlWindow();

		/**
		 *	Shows the information window
		 */
		void showInfoWindow();

		/**
		* Sets the current paint method
		*/
		void selectPaintMethod();

		/**
		*	Sets the current composition cycle mode
		*/
		void selectCompositionCycleMode();

		/**
		*	Sets the current palette week
		*/
		void selectPaletteWeek();

		/**
		*	Sets the current palette cycle mode
		*/
		void selectPaletteCycleMode();

		/**
		*	Sets the current color palette cycle speed
		*/
		void setColorPaletteCycleSpeed(float seconds);

		/**
		 *	Update lux historgram
		 */
		void updateLuxHistogram(double deltaTime);

	private:
		double mElapsedTime = 0.0f;
	};
}
