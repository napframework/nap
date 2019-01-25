// Local Includes
#include "emographyapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <scene.h>
#include <imgui/imgui.h>
#include <inputrouter.h>
#include <emographyreading.h>
#include <emographystressdataviewcomponent.h>
#include <random>
#include <database.h>
#include <utility/fileutils.h>
#include <emographysummaryfunctions.h>
#include <emographystress.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::EmographyApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	using namespace emography;

	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to nap
	 */
	bool EmographyApp::init(utility::ErrorState& error)
	{
		// Create render service
		mRenderService = getCore().getService<RenderService>();

		// Create GUI service
		mGuiService = getCore().getService<IMGuiService>();

		// Create scene service
		mSceneService = getCore().getService<SceneService>();

		// Create input service
		mInputService = getCore().getService<InputService>();

		// Get resource manager service
		mResourceManager = getCore().getResourceManager();

		// Load scene
		if (!mResourceManager->loadFile("emography.json", error))
			return false;

		// Find the window we want to render the output to
		mRenderWindow = mResourceManager->findObject<RenderWindow>("Window");

		mDataModel = std::make_unique<DataModel>(mResourceManager->getFactory());
		if (!mDataModel->init("emography.db", DataModel::EKeepRawReadings::Disabled, error))
			return false;

		bool result = mDataModel->registerType<StressIntensity>(&gAveragingSummary<StressIntensity>, error);

		if (!result)
			return false;

#if 0
		// Get converted time
		DateTime now = getCurrentDateTime();
		auto current_time_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now.getTimeStamp());

		for (int days = 0; days != 7; ++days)
		{
			for (int hours = 0; hours != 24; ++hours)
			{
				Logger::info(utility::stringFormat("%2d:%2d", days, hours));
				for (int minutes = 0; minutes != 60; ++minutes)
				{
					if (hours % 3 == 0 && minutes == 5)
					{
						current_time_ms += Milliseconds(60 * 55 * 1000);
						break;
					}

					float minute_bias = 0.5f + 0.5f * sin(((float)minutes / 60.0f) * math::pi());

					for (int seconds = 0; seconds != 60; ++seconds)
					{
						float seconds_bias = 0.5f + sin(((float)minutes / 60.0f) * math::pi());

						uint64_t num_seconds_samples = 100;
						for (int seconds_samples = 0; seconds_samples != num_seconds_samples; ++seconds_samples)
						{
							utility::ErrorState errorState;

							float value = (float)(rand() % 100) * minute_bias * seconds_bias;

							current_time_ms += Milliseconds(1000 / num_seconds_samples);

							TimeStamp cur_time = TimeStamp(current_time_ms);
							std::unique_ptr<StressIntensityReading> intensityReading = std::make_unique<StressIntensityReading>(value, cur_time);
							if (!mDataModel.add(*intensityReading, errorState))
								nap::Logger::error(errorState.toString());

// 							std::unique_ptr<StressStateReading> stateReading = std::make_unique<StressStateReading>((EStressState)distr(eng), cur_time);
// 							if (!mDataModel.add(*stateReading, errorState))
// 								nap::Logger::error(errorState.toString());
						}
					}
				}
			}
		}
#endif

		return true;
	}

	void EmographyApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		// If we pressed escape, quit the loop
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

			if (press_event->mKey == nap::EKeyCode::KEY_f)
			{
				mRenderWindow->toggleFullscreen();
			}
		}

		mInputService->addEvent(std::move(inputEvent));
	}

	void EmographyApp::renderGUI()
	{
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
		ImGui::Begin("GraphWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_ShowBorders);
		
		glm::vec2 topLeft = glm::vec2(0.0f, 0.0);

		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		ImGui::SliderInt("Resolution", &mResolution, 10, ImGui::GetWindowWidth());
		ImGui::SliderInt("Graph Height", &mGraphHeight, 1, 1000);

		ImGuiIO& io = ImGui::GetIO();

		float timelineHeight = renderTimeline();

		mTimelineState.mWidthInPixels = ImGui::GetWindowWidth();
		
		float height = ImGui::GetWindowHeight() - timelineHeight;
		float y_units = mGraphHeight;

		glm::vec2 mousePos = glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y) - topLeft;
		
		bool mouse_inside_screen = mousePos.x != -FLT_MAX && mousePos.y != -FLT_MAX;		

		if (ImGui::IsMouseClicked(0))
			mTimelineState.OnMouseDown(mousePos, io.KeyCtrl, io.KeyAlt);
		else if (ImGui::IsMouseReleased(0))
			mTimelineState.OnMouseUp(mousePos, io.KeyCtrl, io.KeyAlt);

		if (mouse_inside_screen)
			mTimelineState.OnMouseMove(mousePos, io.KeyCtrl, io.KeyAlt);		

		if (mouse_inside_screen && !mMouseWasInsideScreen)
			mTimelineState.OnMouseEnter(mousePos, io.KeyCtrl, io.KeyAlt);
		else if (!mouse_inside_screen && mMouseWasInsideScreen)
			mTimelineState.OnMouseLeave(mousePos, io.KeyCtrl, io.KeyAlt);

		mMouseWasInsideScreen = mouse_inside_screen;

		int numSamples = mResolution;
		float secondsPerSample = (mTimelineState.mTimeRight - mTimelineState.mTimeLeft) / (float)numSamples;
		if (secondsPerSample < 1.0f)
			numSamples = mTimelineState.mTimeRight - mTimelineState.mTimeLeft;

		TimeStamp left(SystemTimeStamp(Seconds(mTimelineState.mTimeLeft)));
		TimeStamp right(SystemTimeStamp(Seconds(mTimelineState.mTimeRight)));

		std::vector<std::unique_ptr<ReadingSummaryBase>> readings;
		utility::ErrorState errorState;
		if (mDataModel->getRange<StressIntensityReading>(left, right, numSamples, readings, errorState))
		{
			glm::vec2 prevPos(0.0, height);

			float bottomY = height;
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			for (auto& readingBase : readings)
			{
				StressIntensityReadingSummary* reading = rtti_cast<StressIntensityReadingSummary>(readingBase.get());

				float x = mTimelineState.AbsTimeToPixel(reading->mTimeStamp.mTimeStamp / 1000);
				float y = bottomY - glm::clamp(reading->mObject.mValue / y_units, 0.0f, 1.0f) * height;

				draw_list->AddLine(ImVec2(topLeft.x + prevPos.x, topLeft.y + prevPos.y), ImVec2(topLeft.x + x, topLeft.y + y), ImGui::GetColorU32(ImVec4(170.0f / 255.0f, 211.0f / 255.0f, 1.0f, 1.0f)), 1.0f);

				prevPos = glm::vec2(x, y);
			}
		}

		ImGui::End();
	}

	float EmographyApp::renderTimeline()
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		glm::vec2 topLeft = glm::vec2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
		float height = ImGui::GetWindowHeight() - 1;

		double left = mTimelineState.mTimeLeft;
		double right = mTimelineState.mTimeRight;
		double delta = right - left;

		double markerTimelineHeight = 25.0;
		double textTimelineHeight = 25.0f;
		double totalTimelineHeight = markerTimelineHeight + textTimelineHeight + 2;

		// Calculate how many milliseconds a particular unit takes
		const double oneHour = 1000.0 * 60.0 * 60.0;
		const double oneMinute = 1000.0 * 60.0;
		const double oneSecond = 1000.0;
		const double oneMsec = 1.0f;

		struct Separator
		{
			double	mMajorTime;				// Time for major separator
			int		mNumMinorSubdivisions;	// Amount of subdivisons requires for this major separator
		};

		// Table determining where the major separators lie and how the subdivions are placed
		Separator separatorSettings[] =
		{
			{ oneHour,				10 },
			{ oneMinute * 10.0f,	10 },
			{ oneMinute * 5.0f,		5 },
			{ oneMinute,			10 },
			{ oneSecond * 10.0f,	10 },
			{ oneSecond * 5.0f,		5 },
			{ oneSecond,			10 },
			{ oneMsec * 100.0f,		10 },
			{ oneMsec * 50.0f,		5 },
			{ oneMsec * 10.0f,		10 },
			{ oneMsec * 5.0f,		5 },
			{ oneMsec * 1.0f,		10 }
		};

		const int preferredSpaceBetweenSeparators = (int)(300.0f);
		const int preferredNumSeparators = mTimelineState.mWidthInPixels / preferredSpaceBetweenSeparators;

		// Find what major separator comes closest to the space we prefer between separators
		int numMinorSubDivisions = 10;
		double minorStepSize = FLT_MAX;
		double majorStepSize = FLT_MAX;
		for (int i = 0; i != sizeof(separatorSettings) / sizeof(Separator); ++i)
		{
			if (abs((delta / separatorSettings[i].mMajorTime) - preferredNumSeparators) < abs((delta / majorStepSize) - preferredNumSeparators))
			{
				majorStepSize = separatorSettings[i].mMajorTime;
				numMinorSubDivisions = separatorSettings[i].mNumMinorSubdivisions;
				minorStepSize = majorStepSize / (float)separatorSettings[i].mNumMinorSubdivisions;
			}
		}

		// Draw background
		//mCanvas.DrawSolidRect(glm::vec2(0.0f, 0.0f), GetRect().Size(), glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), Renderer::EFillPattern::SolidColor, DrawDepth::sTimelineBackground);

		// Determine if we want to print secs/mins/hour at all. If the time on this bar does not exceed a certain threshold,
		// we don't print that unit at all.
		bool showSecs = (int)(right / oneSecond) > 0;
		bool showMins = (int)(right / oneMinute) > 0;
		bool showHours = (int)(right / oneHour) > 0;

		// Calculate start time value
		double majorMarkerTime = (int)(left / (double)majorStepSize) * majorStepSize;

		// Draw major, minor separators and text
		while (majorMarkerTime < right)
		{
			// Only draw positive values
			if (majorMarkerTime >= 0.0f)
			{
				// Draw major marker line
				int x = mTimelineState.AbsTimeToPixel(majorMarkerTime);
				
				draw_list->AddLine(ImVec2(topLeft.x + x, height - totalTimelineHeight), ImVec2(topLeft.x + x, height - totalTimelineHeight + markerTimelineHeight), ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), 2.0f);

				// Draw all major marker lines
				double minorMarkerTime = majorMarkerTime + minorStepSize;
				for (int minorIndex = 1; minorIndex < numMinorSubDivisions; ++minorIndex)
				{
					float minorX = mTimelineState.AbsTimeToPixel(minorMarkerTime);
					float minorY = markerTimelineHeight * 0.75f;
					if (numMinorSubDivisions == 10 && minorIndex == 5)
						minorY = markerTimelineHeight * 0.5f;

					draw_list->AddLine(ImVec2(topLeft.x + minorX, height - totalTimelineHeight), ImVec2(topLeft.x + minorX, height - totalTimelineHeight + minorY), ImGui::ColorConvertFloat4ToU32(ImVec4(0.5f, 0.5f, 0.5f, 1.0f)), 1.0f);
					minorMarkerTime += minorStepSize;
				}

				SystemTimeStamp majorMarkerTimeStamp(Seconds((uint64_t)majorMarkerTime));
				DateTime majorMarkerDateTime(majorMarkerTimeStamp);

				std::string majorMarkerText = majorMarkerDateTime.toString();
				draw_list->AddText(ImVec2(topLeft.x + x, height - textTimelineHeight), ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), majorMarkerText.c_str());

#if 0
				double remainder = majorMarkerTime;
				int numHours = (int)(remainder / oneHour);
				remainder -= numHours * oneHour;
				int numMins = (int)(remainder / oneMinute);
				remainder -= numMins * oneMinute;
				int numSecs = (int)(remainder / oneSecond);
				remainder -= numSecs * oneSecond;
				int numMsecs = (int)(remainder);

				std::string markerTimeText;

				if (showHours)
					markerTimeText = Utility::FormatString("h%d", numHours);

				if (majorStepSize < oneHour && showMins)
				{
					if (!markerTimeText.empty())
						markerTimeText += ":";
					markerTimeText += Utility::FormatString("m%d", numMins);
				}

				if (majorStepSize < oneMinute && showSecs)
				{
					if (!markerTimeText.empty())
						markerTimeText += ":";
					markerTimeText += Utility::FormatString("s%d", numSecs);
				}

				if (majorStepSize < oneSecond)
				{
					if (!markerTimeText.empty())
						markerTimeText += ":";
					markerTimeText += Utility::FormatString("ms%d", numMsecs);
				}

				mCanvas.DrawText(glm::vec2(x + 10.0f, 6.0f), gGetDefaultTextHeight(), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), markerTimeText, DrawDepth::sTimelineForeground);
#endif
			}

			majorMarkerTime += majorStepSize;
		}
		draw_list->AddLine(ImVec2(topLeft.x, height - totalTimelineHeight), ImVec2(topLeft.x + mTimelineState.mWidthInPixels, height - totalTimelineHeight), ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), 1.0f);

		return totalTimelineHeight;
	}

	void EmographyApp::update(double deltaTime)
	{
		// The default input router forwards messages to key and mouse input components
		// attached to a set of entities.
		nap::DefaultInputRouter input_router;

		// Forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { };
		mInputService->processWindowEvents(*mRenderWindow, input_router, entities);

		renderGUI();
	}
	

	void EmographyApp::render()
	{
		mRenderService->destroyGLContextResources({ mRenderWindow });

		// Make render window active for drawing
		mRenderWindow->makeActive();

		// Clear target
		opengl::RenderTarget& render_target = mRenderWindow->getBackbuffer();
		render_target.setClearColor({ 0.0f, 0.0f, 0.0f, 1.0 });
		mRenderService->clearRenderTarget(render_target);

		// Draw gui
		mGuiService->draw();

		// Swap GPU buffers
		mRenderWindow->swap();
	}

	
	int EmographyApp::shutdown()
	{
		return 0;
	}
}
 
