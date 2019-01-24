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
		for (auto& test : sUnitTests)
		{
			if (!errorState.check(!utility::fileExists(test.mDatabasePath) || utility::deleteFile(test.mDatabasePath), "Failed to delete test database"))
				return false;

			DataModel dataModel;
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

		// Get reference to scene
		mScene = mResourceManager->findObject<Scene>("Scene");

		// Get main entities
		mController = mScene->findEntity("ControllerEntity");
		mSummaryEntity = mScene->findEntity("SummaryEntity");
		mDashboardEntity = mScene->findEntity("DashboardEntity");
		mHistoryEntity = mScene->findEntity("HistoryEntity");

		if (!mDataModel.init("emography.db", DataModel::EKeepRawReadings::Disabled, error))
			return false;

		bool result = mDataModel.registerType<StressIntensity>(&gAveragingSummary<StressIntensity>, error);

		if (!result)
			return false;

// 		result = mDataModel.registerType<EStressState>([](const std::vector<DataModel::WeightedObject>& inObjects)
// 		{
// 			std::unordered_map<EStressState, int> stateCounts;
// 			for (int index = 0; index < inObjects.size(); ++index)
// 			{
// 				rtti::Object* object = inObjects[index].mObject.get();
// 				StressStateReading* stressStateReading = rtti_cast<StressStateReading>(object);
// 				assert(stressStateReading);
// 
// 				stateCounts[stressStateReading->mObject]++;
// 			}
// 
// 			EStressState maxStressState = EStressState::Unknown;
// 			int maxStressStateCount = 0;
// 			for (auto& kvp : stateCounts)
// 			{
// 				if (kvp.second > maxStressStateCount)
// 				{
// 					maxStressStateCount = kvp.second;
// 					maxStressState = kvp.first;
// 				}
// 			}
// 
// 			return std::make_unique<StressStateReading>(maxStressState);
// 		}, error);

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
		/*
		static std::random_device rd; // obtain a random number from hardware
		static std::mt19937 eng(rd()); // seed the generator
		static std::uniform_int_distribution<> distr(-1, 2); // define the range

		static int numAdded = 0;
		static double time = 0;
		time += deltaTime;
		if (time > 1.0f / 60.0f)
		{
			time = 0.0;

			utility::ErrorState errorState;

			std::unique_ptr<StressIntensityReading> intensityReading = std::make_unique<StressIntensityReading>((float)(rand() % 100), cstamp);
			if (!mDataModel.add(*intensityReading, errorState))
				nap::Logger::error(errorState.toString());

			std::unique_ptr<StressStateReading> stateReading = std::make_unique<StressStateReading>((EStressState)distr(eng), cstamp);
			if (!mDataModel.add(*stateReading, errorState))
				nap::Logger::error(errorState.toString());
				}
				*/
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

	struct TimelineState
	{
		enum class EMode : uint8_t
		{
			Select,
			Zoom,
			Pan
		};

		enum class EModeState : uint8_t
		{
			Started,
			Finished
		};

		TimelineState()
		{
			// Get current date (without time)
			std::time_t curtime;
			std::time(&curtime);
			
			std::tm dayStart = *(std::localtime(&curtime));
			dayStart.tm_hour = 0;
			dayStart.tm_min = 0;
			dayStart.tm_sec = 0;

			std::tm dayEnd = *(std::localtime(&curtime));
			dayEnd.tm_hour = 23;
			dayEnd.tm_min = 59;
			dayEnd.tm_sec = 59;

			uint64_t dayStartTime = std::mktime(&dayStart);
			uint64_t dayEndTime = std::mktime(&dayEnd);

			mTimeLeft = dayStartTime;
			mTimeRight = dayEndTime;
		}

		inline float AbsTimeToPixel(uint64_t inTime) const
		{
			double total_time = mTimeRight - mTimeLeft;
			double pixels_per_second = mWidthInPixels / total_time;

			double scroll_offset = mTimeLeft * pixels_per_second;

			return (float)(inTime * pixels_per_second - scroll_offset);
		}

		inline uint64_t PixelToTime(float inXPos) const
		{
			double total_time = mTimeRight - mTimeLeft;
			return (inXPos / mWidthInPixels) * total_time;
		}

		inline uint64_t PixelToAbsTime(float inXPos) const
		{
			uint64_t result = mTimeLeft + PixelToTime(inXPos);
			return glm::clamp(mTimeLeft, mTimeRight, result);
		}

		bool OnMouseDown(const glm::vec2& inPosition, bool inCtrlDown, bool inAltDown)
		{
			//  In case an active mode was set, it means it is a mode set by the UI buttons. We store it here
			// so that we can test later if the mode was overridden by a shortcut key
			mPrevActiveMode = mActiveMode;

			mCursorClickPos = inPosition;

			mDragStartLeftTime = mTimeLeft;
			mDragStartRightTime = mTimeRight;
			mLastCursorTimePos = PixelToAbsTime(inPosition.x);
			mCursorClickTimePos = mLastCursorTimePos;

			// If the active mode is set through the UI buttons, it can be overridden by any of the shortcut keys
			if (inCtrlDown)
				SetActiveMode(EMode::Pan);
			else if (inAltDown)
				SetActiveMode(EMode::Zoom);

			SetModeState(EModeState::Started);

			return true;
		}

		void OnMouseMove(const glm::vec2& inPosition, bool inCtrlDown, bool inAltDown)
		{
			if (IsModeStarted())
			{
				glm::vec2 drag_delta = mCursorClickPos - inPosition;

				// If a button is released, we should cancel and revert back to selection mode. If mPrevActiveMode is set, 
				// it means we entered a mode through the UI buttons. In that case, we should not revert the mode if a 
				// button is released.
				const bool modeActivatedThroughUI = mPrevActiveMode != EMode::Select;

				if (mActiveMode == EMode::Pan)
				{
					if (inCtrlDown || modeActivatedThroughUI)
					{
						double total_time = mDragStartRightTime - mDragStartLeftTime;
						float time_moved = (float)((drag_delta.x / mWidthInPixels) * total_time);

						MoveTo(mDragStartLeftTime + time_moved);
					}
					else
					{
						SetModeState(EModeState::Finished);
						SetActiveMode(EMode::Select);
					}
				}
				else if (mActiveMode == EMode::Zoom)
				{
					if (inAltDown || modeActivatedThroughUI)
					{
						const float mouseMoveMultiplier = 0.001f + powf(0.5f, 3.0f) * 0.05f;
						ZoomAroundTime(mCursorClickTimePos, mDragStartLeftTime, mDragStartRightTime, drag_delta.x * mouseMoveMultiplier);
					}
					else
					{
						SetModeState(EModeState::Finished);
						SetActiveMode(EMode::Select);
					}
				}				
			}

			mLastCursorTimePos = PixelToAbsTime(inPosition.x);
		}

		void OnMouseUp(const glm::vec2& inPosition, bool inCtrlDown, bool inAltDown)
		{
			if (IsModeStarted())
			{
				// If the active mode is overridden by a shortcut, we always go back to select mode.
				if (mActiveMode != mPrevActiveMode)
					SetActiveMode(EMode::Select);

				SetModeState(EModeState::Finished);
			}

			mLastCursorTimePos = PixelToAbsTime(inPosition.x);
		}

		double TimelineState::GetClampedRange(double inTimeRangeMs)
		{
			return glm::clamp<double>(mMinTimeRangeMs, mMaxTimeRangeMs, inTimeRangeMs);
		}

		void ZoomAroundTime(uint64_t inClickTimePos, uint64_t inTimeLeft, uint64_t inTimeRight, float inDelta)
		{
			double exp = std::log10(inTimeRight - inTimeLeft);
			exp += inDelta;
			double range = std::pow(10, exp);
			range = GetClampedRange(range);

			double scale = ((double)inClickTimePos - (double)inTimeLeft) / ((double)inTimeRight - (double)inTimeLeft);

			double newTimeLeft = inClickTimePos - scale * range;
			double newTimeRight = newTimeLeft + range;

			mTimeLeft = newTimeLeft;
			mTimeRight = newTimeRight;
		}

		void MoveTo(uint64_t inStartPosition)
		{
			uint64_t curRange = mDragStartRightTime - mDragStartLeftTime;
			uint64_t endPosition = inStartPosition + curRange;
			uint64_t newStartPosition = inStartPosition;
			uint64_t newEndPosition = endPosition;

			// If one of the sides is clamped, correct the other side
			if (newStartPosition != inStartPosition)
			{
				newEndPosition = newStartPosition + curRange;
			}
			else if (newEndPosition != endPosition)
			{
				newStartPosition = newEndPosition - curRange;
			}

			mTimeLeft = newStartPosition;
			mTimeRight = newEndPosition;
		}

		void SetActiveMode(EMode inMode)
		{
			mActiveMode = inMode;
		}

		void SetModeState(EModeState inModeState)
		{
			mModeState = inModeState;
		}

		bool IsModeStarted() const { return mModeState == EModeState::Started; }

		uint64_t		mTimeLeft = 0;
		uint64_t		mTimeRight = 1000;
		uint64_t		mMinTimeRangeMs = 1;			// 1 sec
		uint64_t		mMaxTimeRangeMs = 60 * 60 * 24;	// 24 hour
		int				mWidthInPixels = 0;

	private:
		EMode			mPrevActiveMode = EMode::Select;
		EMode			mActiveMode = EMode::Select;
		EModeState		mModeState = EModeState::Finished;
		uint64_t		mDragStartLeftTime = 0;
		uint64_t		mDragStartRightTime = 0;
		uint64_t		mSelectedLeftTime = -1;
		uint64_t		mSelectedRightTime = -1;
		uint64_t		mCursorClickTimePos = 0;
		uint64_t		mLastCursorTimePos = -1;
		glm::vec2		mCursorClickPos;
	};

	static TimelineState sTimelineState;

	void EmographyApp::renderGUI()
	{
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
		ImGui::Begin("Test", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_ShowBorders);

		sTimelineState.mWidthInPixels = ImGui::GetWindowWidth();
		float height = ImGui::GetWindowHeight() - 1;
		float y_units = 100;

		glm::vec2 topLeft = glm::vec2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
		glm::vec2 mousePos = glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y) - topLeft;
		ImGuiIO& io = ImGui::GetIO();

		if (ImGui::IsMouseClicked(0))
		{
			sTimelineState.OnMouseDown(mousePos, io.KeyCtrl, io.KeyAlt);
		}
		else if (ImGui::IsMouseReleased(0))
		{
			sTimelineState.OnMouseUp(mousePos, io.KeyCtrl, io.KeyAlt);
		}
		else
		{
			sTimelineState.OnMouseMove(mousePos, io.KeyCtrl, io.KeyAlt);		
		}

		TimeStamp left;
		left.mTimeStamp = sTimelineState.mTimeLeft * 1000;

		TimeStamp right;
		right.mTimeStamp = sTimelineState.mTimeRight * 1000;


		int numSamples = 400;
		float secondsPerSample = (sTimelineState.mTimeRight - sTimelineState.mTimeLeft) / (float)numSamples;
		if (secondsPerSample < 1.0f)
		{
			numSamples = sTimelineState.mTimeRight - sTimelineState.mTimeLeft;
		}

		std::vector<std::unique_ptr<ReadingSummaryBase>> readings;
		utility::ErrorState errorState;
		if (mDataModel.getRange<StressIntensityReading>(left, right, numSamples, readings, errorState))
		{
			glm::vec2 prevPos(0.0, height);

			float bottomY = height;
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			for (auto& readingBase : readings)
			{
				StressIntensityReadingSummary* reading = rtti_cast<StressIntensityReadingSummary>(readingBase.get());

				float x = sTimelineState.AbsTimeToPixel(reading->mTimeStamp.mTimeStamp / 1000);
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

		// Get current date time
		DateTime now = getCurrentDateTime();

		// Compute time range
		SystemTimeStamp today = now.getTimeStamp();
		SystemTimeStamp yeste = today - std::chrono::hours(24);

		// Set
		StressDataViewComponentInstance& stress_comp = mHistoryEntity->getComponent<StressDataViewComponentInstance>();
		stress_comp.setTimeRange(yeste, today);

		//////////////////////////////////////////////////////////////////////////
		// Date time conversion test
		//////////////////////////////////////////////////////////////////////////
		
		// Create snapshot from datetime
		StressStateReading snapshot(nap::emography::EStressState::Over, now.getTimeStamp());
		/*
		// Get converted time
		SystemTimeStamp cstamp = snapshot.mTimeStamp.toSystemTime();
		DateTime con(cstamp, DateTime::ConversionMode::Local);

		static std::random_device rd; // obtain a random number from hardware
		static std::mt19937 eng(rd()); // seed the generator
		static std::uniform_int_distribution<> distr(-1, 2); // define the range

		static int numAdded = 0;
		static double time = 0;
		time += deltaTime;
		if (time > 1.0f/60.0f)
		{
			time = 0.0;

			utility::ErrorState errorState;

			std::unique_ptr<StressIntensityReading> intensityReading = std::make_unique<StressIntensityReading>((float)(rand() % 100), cstamp);
			if (!mDataModel.add(*intensityReading, errorState))
				nap::Logger::error(errorState.toString());

			std::unique_ptr<StressStateReading> stateReading = std::make_unique<StressStateReading>((EStressState)distr(eng), cstamp);
			if (!mDataModel.add(*stateReading, errorState))
				nap::Logger::error(errorState.toString());
		}*/

		renderGUI();
		//updateGui();
	}


// 
// 	void BarView::RenderBarsRecursive(BarAccumulator& inAccumulator, const BarNodeList& inBars, float inLeft, float inRight, float inCurrentYPos, float& outMaxYPos)
// 	{
// 		ImDrawList* draw_list = ImGui::GetWindowDrawList();
// 
// 		float window_width = inRight - inLeft;
// 
// 		// Determine some conversion factors
// 		double total_time = mTimeRight.GetValue() - mTimeLeft.GetValue();
// 		double pixels_per_second = window_width / total_time;
// 
// 		// Determine scroll offset
// 		float scroll_offset = (float)(mTimeLeft.GetValue() * pixels_per_second);
// 
// 		// Find bars in range
// 		BarRange range(inBars, mTimeLeft.GetValue(), mTimeRight.GetValue());
// 
// 		// Render bars at this level first
// 		for (auto& bar : range)
// 		{
// 			double bar_start_time = bar->mBar.mStartTimeMs;
// 			double bar_end_time = bar->mBar.mEndTimeMs;
// 
// 			// Convert bar start time to pixels
// 			float bar_start_pixels = (float)(bar_start_time * pixels_per_second) - scroll_offset;
// 			bar_start_pixels = Math::gMax(inLeft + bar_start_pixels, inLeft);
// 
// 			// Convert bar end time to pixels
// 			float bar_end_pixels = (float)(bar_end_time * pixels_per_second) - scroll_offset;
// 			bar_end_pixels = Math::gMin(inLeft + bar_end_pixels, inRight);
// 
// 			// TODO: determine bar color
// 			glm::vec3 bar_color(0.0f, 1.0f, 0.0f);
// 
// 			// Add bar to accumulator
// 			inAccumulator.AddBorderedBar(bar_start_pixels, bar_end_pixels, bar_color);
// 
// 			// Render bar
// 			// 			draw_list->AddQuadFilled(glm::vec2(bar_start_pixels, currentY), glm::vec2(bar_end_pixels, currentY), 
// 			// 				glm::vec2(bar_end_pixels, currentY + sBarHeight), glm::vec2(bar_start_pixels, currentY + sBarHeight), 
// 			// 				ImGui::GetColorU32(glm::vec4(bar_color, 1.0f)));
// 
// 			if (bar_end_pixels - bar_start_pixels > 15)
// 			{
// 				std::string text = mStringTable->Find(bar->mBar.mStringID);
// 				draw_list->PushClipRect(glm::vec2(bar_start_pixels + 5, inCurrentYPos), glm::vec2(bar_end_pixels - 5, inCurrentYPos + sBarHeight));
// 				draw_list->AddText(glm::vec2(bar_start_pixels + 5, inCurrentYPos), ImGui::GetColorU32(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), text.c_str());
// 				draw_list->PopClipRect();
// 			}
// 
// 			if ((inCurrentYPos + sBarHeight) > outMaxYPos)
// 				outMaxYPos = inCurrentYPos + sBarHeight;
// 		}
// 
// 		// Flush any remaining pixels
// 		inAccumulator.Flush();
// 
// 		// Now recurse
// 		BarAccumulator child_accumulator(inCurrentYPos + sBarHeight + sBarGap, inCurrentYPos + sBarHeight + sBarGap + sBarHeight);
// 		for (auto& bar : range)
// 		{
// 			RenderBarsRecursive(child_accumulator, bar->mChildNodes, inLeft, inRight, inCurrentYPos + sBarHeight + sBarGap, outMaxYPos);
// 		}
// 	}

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
 
