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
		mSceneService	= getCore().getService<SceneService>();

		// Create input service
		mInputService = getCore().getService<InputService>();

		// Get resource manager service
		mResourceManager = getCore().getResourceManager();

		// Load scene
		if (!mResourceManager->loadFile("emography.json", error)) 
			return false;    

		// Find the window we want to render the output to
		mRenderWindow = mResourceManager->findObject<RenderWindow>("Window");

		// Find the data model
		mDataModel = mResourceManager->findObject<emography::DataModel>("DataModel");

		// Register type to read
		if (!mDataModel->getInstance().registerType<StressIntensity>(&gAveragingSummary<StressIntensity>, error))
			return false;

		return true;
	}
	
	
	void EmographyApp::clearData()
	{
		utility::ErrorState errorState;
		if (!mDataModel->getInstance().clearData<StressIntensityReading>(errorState))
			Logger::error(utility::stringFormat("Failed to clear data: %s", errorState.toString().c_str()));
	}


	void EmographyApp::generateData(int numDays)
	{
		// Clear existing data
		clearData();

		// Start generating data from the last timestamp in the model, if available. Otherwise generate data starting at the current time.
		SystemTimeStamp current_time = getCurrentTime();// - (Hours(24) * numDays);
		TimeStamp generate_start = TimeStamp(current_time);
		int num_samples_added = 0;
		uint64_t num_second_samples = 1;

		// Used for tracking progress (0-100)
		int step_inc = (numDays * 24 * 60 * 60 * num_second_samples) / 100;
		int curr_pro = 0;
		int next_inc = step_inc;

		for (int days = 0; days != numDays; ++days)
		{
			for (int hours = 0; hours != 24; ++hours)
			{
				for (int minutes = 0; minutes != 60; ++minutes)
				{
					float minute_bias = 0.5f + 0.5f * sin(((float)minutes / 60.0f) * math::pi());
					for (int seconds = 0; seconds != 60; ++seconds)
					{
						float seconds_bias = 0.5f + sin(((float)minutes / 60.0f) * math::pi());

						for (int seconds_samples = 0; seconds_samples != num_second_samples; ++seconds_samples)
						{
							utility::ErrorState errorState;
							float value = (float)(rand() % 100) * minute_bias * seconds_bias;
							current_time += Milliseconds(1000 / num_second_samples);

							std::unique_ptr<StressIntensityReading> intensityReading = std::make_unique<StressIntensityReading>(value, current_time);
							if (!mDataModel->getInstance().add(*intensityReading, errorState))
							{
								nap::Logger::error(errorState.toString());
								return;
							}
							num_samples_added++;

							// Report progress
							if (num_samples_added == next_inc)
							{
								next_inc += step_inc;
								Logger::info(utility::stringFormat("%d", ++curr_pro));
							}
						}
					}
				}
			}
		}

		TimeStamp generate_end = getCurrentTime();
		double generate_ms = (double)(std::chrono::time_point_cast<Milliseconds>(generate_end.toSystemTime()).time_since_epoch().count() - std::chrono::time_point_cast<Milliseconds>(generate_start.toSystemTime()).time_since_epoch().count());
		Logger::info("Generating %d days of data (%d samples) took %.2fs (%fs/sample)", numDays, num_samples_added, generate_ms / 1000.0, generate_ms / 1000.0 / num_samples_added);
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

		updateTimelineState();
		renderControls();

		float timeline_height = renderTimeline();	

		renderGraph(ImGui::GetWindowHeight() - timeline_height);

		ImGui::End();
	}


	void EmographyApp::updateTimelineState()
	{
		ImGuiIO& io = ImGui::GetIO();
		mTimelineState.mWidthInPixels = ImGui::GetWindowWidth();

		glm::vec2 mouse_pos = glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y);

		bool mouse_inside_screen = mouse_pos.x != -FLT_MAX && mouse_pos.y != -FLT_MAX;

		if (ImGui::IsMouseClicked(0))
			mTimelineState.OnMouseDown(mouse_pos, io.KeyCtrl, io.KeyAlt);
		else if (ImGui::IsMouseReleased(0))
			mTimelineState.OnMouseUp(mouse_pos, io.KeyCtrl, io.KeyAlt);

		if (mouse_inside_screen)
			mTimelineState.OnMouseMove(mouse_pos, io.KeyCtrl, io.KeyAlt);

		if (mouse_inside_screen && !mMouseWasInsideScreen)
			mTimelineState.OnMouseEnter(mouse_pos, io.KeyCtrl, io.KeyAlt);
		else if (!mouse_inside_screen && mMouseWasInsideScreen)
			mTimelineState.OnMouseLeave(mouse_pos, io.KeyCtrl, io.KeyAlt);

		mMouseWasInsideScreen = mouse_inside_screen;
	}


	void EmographyApp::renderControls()
	{
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		ImGui::NewLine();

		ImGui::BeginChild("Controls", ImVec2(300, 200), false);

		if (ImGui::Button("Clear Data"))
			clearData();

		ImGui::SameLine();

		if (ImGui::Button("Generate data"))
			generateData(mNumDaysToGenerate);

		ImGui::InputInt("Days", &mNumDaysToGenerate, 0, 0);
		ImGui::NewLine();

		ImGui::Text("Render options");
		ImGui::SliderInt("Resolution", &mResolution, 10, 1000);
		ImGui::SliderInt("Graph Height", &mGraphYUnits, 1, 1000);

		ImGui::EndChild();
	}


	float EmographyApp::renderTimeline()
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		glm::vec2 top_left = glm::vec2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
		float height = ImGui::GetWindowHeight() - 1;

		double left = mTimelineState.mTimeLeft;
		double right = mTimelineState.mTimeRight;
		double delta = right - left;

		double marker_timeline_height = 25.0;
		double text_timeline_height = 25.0f;
		double total_timeline_height = marker_timeline_height + text_timeline_height + 2;

		// Calculate how many milliseconds a particular unit takes
		const double one_hour = 1000.0 * 60.0 * 60.0;
		const double one_minute = 1000.0 * 60.0;
		const double one_second = 1000.0;
		const double one_msec = 1.0f;

		struct Separator
		{
			double	mMajorTime;				// Time for major separator
			int		mNumMinorSubdivisions;	// Amount of subdivisions requires for this major separator
		};

		// Table determining where the major separators lie and how the subdivisions are placed
		Separator separator_settings[] =
		{
			{ one_hour,				10 },
			{ one_minute * 10.0f,	10 },
			{ one_minute * 5.0f,		5 },
			{ one_minute,			10 },
			{ one_second * 10.0f,	10 },
			{ one_second * 5.0f,		5 },
			{ one_second,			10 },
			{ one_msec * 100.0f,		10 },
			{ one_msec * 50.0f,		5 },
			{ one_msec * 10.0f,		10 },
			{ one_msec * 5.0f,		5 },
			{ one_msec * 1.0f,		10 }
		};

		const int preferred_space_between_separators = (int)(300.0f);
		const int preferred_num_separators = mTimelineState.mWidthInPixels / preferred_space_between_separators;

		// Find what major separator comes closest to the space we prefer between separators
		int num_minor_sub_divisions = 10;
		double minor_step_size = FLT_MAX;
		double major_step_size = FLT_MAX;
		for (int i = 0; i != sizeof(separator_settings) / sizeof(Separator); ++i)
		{
			if (abs((delta / separator_settings[i].mMajorTime) - preferred_num_separators) < abs((delta / major_step_size) - preferred_num_separators))
			{
				major_step_size = separator_settings[i].mMajorTime;
				num_minor_sub_divisions = separator_settings[i].mNumMinorSubdivisions;
				minor_step_size = major_step_size / (float)separator_settings[i].mNumMinorSubdivisions;
			}
		}

		// Calculate start time value
		double major_marker_time = (int)(left / (double)major_step_size) * major_step_size;

		// Draw major, minor separators and text
		while (major_marker_time < right)
		{
			// Only draw positive values
			if (major_marker_time >= 0.0f)
			{
				// Draw major marker line
				int x = mTimelineState.AbsTimeToPixel(major_marker_time);
				
				draw_list->AddLine(ImVec2(top_left.x + x, height - total_timeline_height), ImVec2(top_left.x + x, height - total_timeline_height + marker_timeline_height), ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), 2.0f);

				// Draw all major marker lines
				double minor_marker_time = major_marker_time + minor_step_size;
				for (int minor_index = 1; minor_index < num_minor_sub_divisions; ++minor_index)
				{
					float minor_x = mTimelineState.AbsTimeToPixel(minor_marker_time);
					float minor_y = marker_timeline_height * 0.75f;
					if (num_minor_sub_divisions == 10 && minor_index == 5)
						minor_y = marker_timeline_height * 0.5f;

					draw_list->AddLine(ImVec2(top_left.x + minor_x, height - total_timeline_height), ImVec2(top_left.x + minor_x, height - total_timeline_height + minor_y), ImGui::ColorConvertFloat4ToU32(ImVec4(0.5f, 0.5f, 0.5f, 1.0f)), 1.0f);
					minor_marker_time += minor_step_size;
				}

				SystemTimeStamp major_marker_time(Seconds((uint64_t)major_marker_time));
				DateTime major_marker_datetime(major_marker_time);

				std::string major_marker_text = major_marker_datetime.toString();
				draw_list->AddText(ImVec2(top_left.x + x, height - text_timeline_height), ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), major_marker_text.c_str());
			}

			major_marker_time += major_step_size;
		}
		draw_list->AddLine(ImVec2(top_left.x, height - total_timeline_height), ImVec2(top_left.x + mTimelineState.mWidthInPixels, height - total_timeline_height), ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), 1.0f);

		return total_timeline_height;
	}


	void EmographyApp::renderGraph(float graphHeight)
	{
		float y_units = mGraphYUnits;

		int num_samples = mResolution;
		float seconds_per_sample = (mTimelineState.mTimeRight - mTimelineState.mTimeLeft) / (float)num_samples;
		if (seconds_per_sample < 1.0f)
			num_samples = mTimelineState.mTimeRight - mTimelineState.mTimeLeft;

		TimeStamp left(SystemTimeStamp(Seconds(mTimelineState.mTimeLeft)));
		TimeStamp right(SystemTimeStamp(Seconds(mTimelineState.mTimeRight)));

		std::vector<std::unique_ptr<ReadingSummaryBase>> readings;
		utility::ErrorState errorState;
		if (mDataModel->getInstance().getRange<StressIntensityReading>(left, right, num_samples, readings, errorState))
		{
			glm::vec2 prev_pos(0.0, graphHeight);

			float bottom_y = graphHeight;
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			for (auto& reading_base : readings)
			{
				StressIntensityReadingSummary* reading = rtti_cast<StressIntensityReadingSummary>(reading_base.get());
				float x = mTimelineState.AbsTimeToPixel(reading->mTimeStamp.mTimeStamp / 1000);
				float y = bottom_y - glm::clamp(reading->mObject.mValue / y_units, 0.0f, 1.0f) * graphHeight;

				draw_list->AddLine(ImVec2(prev_pos.x, prev_pos.y), ImVec2(x, y), ImGui::GetColorU32(ImVec4(170.0f / 255.0f, 211.0f / 255.0f, 1.0f, 1.0f)), 1.0f);

				prev_pos = glm::vec2(x, y);
			}
		}
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
 
