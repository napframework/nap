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
#include <parametervec.h>
#include <sequenceplayercomponent.h>
#include <oscinputcomponent.h>
#include <maccontroller.h>
#include <nap/timer.h>

#include "flexblockcomponent.h"
#include "sequenceplayercomponent.h"

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

		void initOscInputs();

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

		void showTimeLineWindow();

		void handleInsertionPopup();

		void drawTimeline(bool & outPopupOpened, std::string & popupId);

		void drawTimelinePlayerControls(bool & outPopupOpened, std::string & popupId);

		void handleLoadPopup();

		void handleSequenceActionsPopup();

		void handleSaveAsPopup();

	private:
		FlexblockApp&						mApp;				///< The actual atmos application we build the gui for
		ParameterService&					mParameterService;
		std::unique_ptr<ParameterGUI>		mParameterGUI;
		bool								mHide = false;
		bool								mShowTimeLine = false;
		bool								mShowSequenceList = false;
		DateTime							mDateTime;
		RGBColor8							mTextColor = { 0xC8, 0x69, 0x69 };
		float								mTexPreviewDisplaySize = 1.0f;
		float								mWraPreviewDisplaySize = 1.0f;
		float								mVidPreviewDisplaySize = 1.0f;
		float								mScrub = 0.0f;
		nap::SystemTimer					mTimer;
		float								mUpdateTime = 0.0f;
		float								vel = 0.0f;
		float								tor = 0.0f;

		timeline::SequencePlayerComponentInstance* mSequencePlayer = nullptr;
		FlexBlockComponentInstance* mFlexBlock = nullptr;

		std::vector<OSCInputComponentInstance*> mOscInputs;
		std::vector<ParameterFloat*> mParameters;

		ResourcePtr<MACController> mController;

		/**
		 * Shows the information window
		 */
		void showInfoWindow();

		bool handleNewShowPopup(std::string & outNewFilename, utility::ErrorState& error);

		std::string formatTimeString(float time);

		void showSequencesWindow();

		void handleElementActionsPopup();

		void initParameters();
		void updateInput(int index, float value);

		bool insertNewElement(std::unique_ptr<timeline::SequenceElement> newElement, utility::ErrorState errorState);

		bool insertNewSequence(std::unique_ptr<timeline::Sequence> newSequence, utility::ErrorState errorState);

		template<typename T1>
		std::string convertToString(T1 number, int precision);

		std::string getTimeString();
	};
}