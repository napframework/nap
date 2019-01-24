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

	static bool sInitDataModel(DataModel& dataModel, const std::string& path, utility::ErrorState& errorState)
	{
		if (!dataModel.init(path, DataModel::EKeepRawReadings::Disabled, errorState))
			return false;

		bool result = dataModel.registerType<StressIntensity>(&gAveragingSummary<StressIntensity>, errorState);

		return result;
	}

	struct UnitTestDescriptor
	{
		typedef bool(*UnitTestFunc)(DataModel& dataModel, utility::ErrorState& errorState);
		std::string mDatabasePath;
		UnitTestFunc mTestFunc;
	};

	static bool sRunUnitTestSingleSample(DataModel& dataModel, utility::ErrorState& errorState)
	{
		TimeStamp now = getCurrentTime();

		float value = 50.0f;

		std::unique_ptr<StressIntensityReading> intensityReading = std::make_unique<StressIntensityReading>(value, now);
		if (!dataModel.add(*intensityReading, errorState))
			return false;

		if (!dataModel.flush(errorState))
			return false;

		std::vector<std::unique_ptr<ReadingSummaryBase>> objects;
		if (!dataModel.getRange<StressIntensityReading>(now, now.toSystemTime() + Milliseconds(1000), 1, objects, errorState))
			return false;

		if (!errorState.check(objects.size() == 1, "Expected one second of data"))
			return false;

		StressIntensityReadingSummary* reading = rtti_cast<StressIntensityReadingSummary>(objects[0].get());
		if (!errorState.check(reading != nullptr, "Expected StressIntensityReading object"))
			return false;

		if (!errorState.check(reading->mObject.mValue == value, "Incorrect value returned"))
			return false;

		if (!errorState.check(reading->mNumSecondsActive == 1, "Expected a single second of active data"))
			return false;

		return true;
	}

	static bool sRunUnalignedMinuteTest(DataModel& dataModel, utility::ErrorState& errorState)
	{
		SystemTimeStamp time(Seconds(0));
		SystemTimeStamp start = time;

		// Create three minutes of data
		for (int i = 0; i != 180; ++i)
		{
			std::unique_ptr<StressIntensityReading> intensityReading = std::make_unique<StressIntensityReading>(i, time);
			if (!dataModel.add(*intensityReading, errorState))
				return false;

			time += Seconds(1);
		}

		if (!dataModel.flush(errorState))
			return false;

		// Request a one and a half minute of unaligned data
		const int num_seconds = 90;
		for (int start_second = 30; start_second != 60; ++start_second)
		{
			// Calc average
			int total = 0;
			for (int i = 0; i != num_seconds; ++i)
				total += start_second + i;

			float average = total / (float)num_seconds;

			std::vector<std::unique_ptr<ReadingSummaryBase>> objects;
			if (!dataModel.getRange<StressIntensityReading>(start + Seconds(start_second), start + Seconds(start_second + num_seconds), 1, objects, errorState))
				return false;

			if (!errorState.check(objects.size() == 1, "Expected one value"))
				return false;

			StressIntensityReadingSummary* reading = rtti_cast<StressIntensityReadingSummary>(objects[0].get());
			if (!errorState.check(reading != nullptr, "Expected StressIntensityReading object"))
				return false;

			if (!errorState.check(std::abs(reading->mObject.mValue - average) < 0.001, "Incorrect value returned"))
				return false;

			if (!errorState.check(reading->mNumSecondsActive == num_seconds, "Expected a single second of active data"))
				return false;
		}

		return true;
	}

	static bool sRunAlignedMinuteTest(DataModel& dataModel, utility::ErrorState& errorState)
	{
		float value = 50.0f;

		SystemTimeStamp time(Seconds(0));
		SystemTimeStamp start = time;

		int total = 0;
		for (int i = 0; i != 60; ++i)
		{
			total += i;
			std::unique_ptr<StressIntensityReading> intensityReading = std::make_unique<StressIntensityReading>(i, time);
			if (!dataModel.add(*intensityReading, errorState))
				return false;

			time += Seconds(1);
		}
		float average = total / 60.0f;

		if (!dataModel.flush(errorState))
			return false;

		std::vector<std::unique_ptr<ReadingSummaryBase>> objects;
		if (!dataModel.getRange<StressIntensityReading>(start, start + Seconds(60), 1, objects, errorState))
			return false;

		if (!errorState.check(objects.size() == 1, "Expected one value"))
			return false;

		StressIntensityReadingSummary* reading = rtti_cast<StressIntensityReadingSummary>(objects[0].get());
		if (!errorState.check(reading != nullptr, "Expected StressIntensityReading object"))
			return false;

		if (!errorState.check(std::abs(reading->mObject.mValue - average) < 0.001, "Incorrect value returned"))
			return false;

		if (!errorState.check(reading->mNumSecondsActive == 60, "Expected a single second of active data"))
			return false;

		return true;
	}

	static bool sRunInactivityTest(DataModel& dataModel, utility::ErrorState& errorState)
	{
		SystemTimeStamp time(Seconds(0));
		SystemTimeStamp start = time;

		int total_active = 0;
		int active_count = 0;

		bool active = true;
		int num_inactive_samples = 0;

		struct Average
		{
			int mCount = 0;
			int mTotal = 0;
		};

		const int num_minutes = 3;
		Average minute_averages[num_minutes];

		// Create three minutes of data
		for (int i = 0; i != num_minutes * 60; ++i)
		{
			if (i % 10 != 0)
				active = !active;
			
			if (active)
			{
				std::unique_ptr<StressIntensityReading> intensityReading = std::make_unique<StressIntensityReading>(i, time);
				if (!dataModel.add(*intensityReading, errorState))
					return false;

				total_active += i;
				active_count++;

				int currentMinute = i / 60;
				minute_averages[currentMinute].mTotal += i;
				minute_averages[currentMinute].mCount++;
			}

			time += Seconds(1);
		}

		float active_average = (float)total_active / (float)active_count;

		if (!dataModel.flush(errorState))
			return false;

		// Single value test
		{
			std::vector<std::unique_ptr<ReadingSummaryBase>> objects;
			if (!dataModel.getRange<StressIntensityReading>(start, start + Seconds(num_minutes * 60), 1, objects, errorState))
				return false;

			if (!errorState.check(objects.size() == 1, "Expected one second of data"))
				return false;

			StressIntensityReadingSummary* reading = rtti_cast<StressIntensityReadingSummary>(objects[0].get());
			if (!errorState.check(reading != nullptr, "Expected StressIntensityReading object"))
				return false;

			if (!errorState.check(std::abs(reading->mObject.mValue - active_average) < 0.001, "Incorrect value returned"))
				return false;

			if (!errorState.check(reading->mNumSecondsActive == active_count, "Wrong number of active seconds returned"))
				return false;
		}

		// Multi value test
		{
			std::vector<std::unique_ptr<ReadingSummaryBase>> objects;
			if (!dataModel.getRange<StressIntensityReading>(start, start + Seconds(num_minutes * 60), num_minutes, objects, errorState))
				return false;

			if (!errorState.check(objects.size() == num_minutes, "Expected %d items of data", num_minutes))
				return false;

			for (int i = 0; i < num_minutes; ++i)
			{
				float minute_average = (float)minute_averages[i].mTotal / (float)minute_averages[i].mCount;

				StressIntensityReadingSummary* reading = rtti_cast<StressIntensityReadingSummary>(objects[i].get());
				if (!errorState.check(reading != nullptr, "Expected StressIntensityReading object"))
					return false;

				if (!errorState.check(std::abs(reading->mObject.mValue - minute_average) < 0.001, "Incorrect value returned"))
					return false;

				if (!errorState.check(reading->mNumSecondsActive == minute_averages[i].mCount, "Wrong number of active seconds returned"))
					return false;
			}
		}
		return true;
	}

	std::vector<UnitTestDescriptor> sUnitTests = { 
		{ "SingleValueTest.db", &sRunUnitTestSingleSample },
		{ "AlignedMinuteTest.db", &sRunAlignedMinuteTest },
		{ "UnalignedMinuteTest.db", &sRunUnalignedMinuteTest },
		{ "InactivityTest.db", &sRunInactivityTest }
	};

	static bool sRunUnitTests(utility::ErrorState& errorState)
	{
		rtti::Factory factory;
		for (auto& test : sUnitTests)
		{
			if (!errorState.check(!utility::fileExists(test.mDatabasePath) || utility::deleteFile(test.mDatabasePath), "Failed to delete test database"))
				return false;

			DataModel dataModel(factory);
			if (!sInitDataModel(dataModel, test.mDatabasePath, errorState))
				return false;

			if (!test.mTestFunc(dataModel, errorState))
				return false;
		}

		return true;
	}

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

		if (!sRunUnitTests(error))
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
		ImGui::Begin("GraphWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_ShowBorders);
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());

		ImGuiIO& io = ImGui::GetIO();

		mTimelineState.mWidthInPixels = ImGui::GetWindowWidth();
		
		float height = ImGui::GetWindowHeight() - 1;
		float y_units = 100;

		glm::vec2 topLeft = glm::vec2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
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

		int numSamples = 400;
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

				draw_list->AddLine(ImVec2(topLeft.x + prevPos.x, topLeft.y + prevPos.y), ImVec2(topLeft.x + x, topLeft.y + y), ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), 1.0f);

				prevPos = glm::vec2(x, y);
			}
		}

		ImGui::End();
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
 
