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

#include "flexblockcomponent.h"
#include "sequenceplayercomponent.h"
#include "timelineguiproperties.h"

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

		void initOscInputs();

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

		/**
		 * Sets the window size, use for calculating positions of imgui windows
		 */
		void setWindowSize(glm::vec2 size) { mWindowSize = size; }
	private:
		FlexblockApp&						mApp;				///< The actual flexblock application we build the gui for
		ParameterService&					mParameterService;
		std::unique_ptr<ParameterGUI>		mParameterGUI;
		bool								mHide = false;
		bool								mShowTimeLine = false;
		bool								mShowPlaylist = false;
		DateTime							mDateTime;
		double								mTime = 0.0f;

		timeline::SequencePlayerComponentInstance* mSequencePlayer = nullptr;
		FlexBlockComponentInstance* mFlexBlock = nullptr;

		std::vector<OSCInputComponentInstance*> mOscInputs;
		std::vector<ParameterFloat*> mParameters;

		TimelineGuiProperties mProps;

		/**
		 * Shows the information window
		 */
		void showInfoWindow();

		bool handleNewShowPopup(std::string & outNewFilename, utility::ErrorState& error);

		std::string formatTimeString(float time);

		void showPlaylist();

		void handleElementActionsPopup();

		/**
		* show the timeline window
		*/
		void showTimeLineWindow();

		/**
		* Handle insertion popup
		*/
		void handleInsertionPopup();

		/**
		* Draw timeline
		*/
		void drawTimeline(bool & outPopupOpened, std::string & popupId);

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

		bool insertNewElement(std::unique_ptr<timeline::SequenceElement> newElement, utility::ErrorState errorState);

		bool insertNewSequence(std::unique_ptr<timeline::Sequence> newSequence, utility::ErrorState errorState);

		template<typename T1>
		std::string convertToString(T1 number, int precision);

		std::string getTimeString();

		glm::vec2 mWindowSize;
	};
}