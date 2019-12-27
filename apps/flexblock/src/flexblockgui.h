#pragma once

// Local Includes
#include "timelineguiproperties.h"

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
#include <oscsender.h>
#include <maccontroller.h>

#include <flexblockcomponent.h>
#include <sequenceplayercomponent.h>
#include <macadapter.h>

namespace nap
{
	// Forward Declares
	class FlexblockApp;
	class ParameterGUI;
	class ParameterService;
	class TimelineGuiProperties;

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
		void update(double time);

		/**
		 *	Render the gui
		 */
		void render();

		/**
		 *	Toggles GUI visibility
		 */
		void toggleVisibility();

		void toggleEditableTimelines();

		/**
		 * Sets the window size, use for calculating positions of imgui windows
		 */
		void setWindowSize(glm::vec2 size) { mWindowSize = size; }

		/**
		 * 
		 */
		void shutDown();

		/**
		 * 
		 */
		void quitDialog();

		void emergencyStop();
	private:
		FlexblockApp&						mApp;					///< The actual flexblock application we build the gui for
		ParameterService&					mParameterService;
		std::unique_ptr<ParameterGUI>		mParameterGUI;
		bool								mHide = false;
		DateTime							mDateTime;
		double								mTime = 0.0f;
		nap::ResourcePtr<MACController>		mMotorController;
		nap::ResourcePtr<MACAdapter>		mMotorAdapter;
		int									mResetMotorPos = 0;
		RGBColor8							mTextColor = { 0xC8, 0x69, 0x69 };
		std::vector<float>					mTargetMeters = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		bool								mEditAllTimelines = false;

		bool								mTimeJumpShouldPlayAfterTransitionDone = false;
		bool								mInTimeJumpTransition = false;
		double								mTimeJumpSequencePlayerTarget = 0.0;
		float								mTimeJumpDifferenceThreshold = 10.0f;

		timeline::SequencePlayerComponentInstance* mSequencePlayer = nullptr;
		FlexBlockComponentInstance* mFlexBlock = nullptr;
		std::vector<ParameterFloat*> mParameters;
		TimelineGuiProperties mProps;

		/**
		 * Shows the information window
		 */
		void showInfoWindow();

		/**
		 * Shows motor control window
		 */
		void showMotorControlWindow();

		bool handleNewShowPopup(std::string & outNewFilename, utility::ErrorState& error);

		std::string formatTimeString(float time);

		void drawPlaylist(bool& outOpenPopup, std::string & outPopupID);

		void handleElementActionsPopup();

		/**
		* show the timeline window
		*/
		void showTimeLineWindow();

		void handleQuitPopup();

		/**
		* Handle insertion popup
		*/
		void handleInsertionPopup();

		void handleEditValuePopup();

		/**
		* Draw timeline
		*/
		void drawTimeline(bool & outPopupOpened, std::string & popupId, std::string timelineId, int size, int offset);

		/**
		* Draw controls
		*/
		void drawTimelinePlayerControls(bool & outPopupOpened, std::string & popupId);

		/**
		* handle show load popup
		*/
		void handleLoadPopup();

		/**
		* handle sequence actions popup
		*/
		void handleSequenceActionsPopup();

		/**
		* Handle save popup
		*/
		void handleSaveAsPopup();

		void initParameters();

		void updateInput(int index, float value);

		void updateOverride(int index, float value);

		bool insertNewElement(std::unique_ptr<timeline::SequenceElement> newElement, utility::ErrorState& errorState);

		bool insertNewSequence(std::unique_ptr<timeline::Sequence> newSequence, utility::ErrorState& errorState);

		bool handleTimeJump(const double time);

		void handleTimeJumpPopup();

		

		template<typename T1>
		std::string convertToString(T1 number, int precision);

		std::string getTimeString();

		void showMotorSteps();

		glm::vec2 mWindowSize;
	};
}