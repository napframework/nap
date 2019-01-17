// Local Includes
#include "emographyapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <scene.h>
#include <imgui/imgui.h>
#include <inputrouter.h>
#include <emographysnapshot.h>
#include <emographystressdataviewcomponent.h>
#include <random>
#include <database.h>

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

		// Get reference to scene
		mScene = mResourceManager->findObject<Scene>("Scene");

		// Get main entities
		mController = mScene->findEntity("ControllerEntity");
		mSummaryEntity = mScene->findEntity("SummaryEntity");
		mDashboardEntity = mScene->findEntity("DashboardEntity");
		mHistoryEntity = mScene->findEntity("HistoryEntity");

		if (!mDataModel.init(error))
			return false;		

		bool result = mDataModel.registerType<StressIntensity>([](const std::vector<std::unique_ptr<rtti::Object>>& inObjects) 
		{
			float total = 0.0;
			TimeStamp startTime;
			TimeStamp endTime;
			for (int index = 0; index < inObjects.size(); ++index)
			{
				rtti::Object* object = inObjects[index].get();
				StressIntensityReading* stressIntensityReading = rtti_cast<StressIntensityReading>(object);
				assert(stressIntensityReading);

				if (index == 0)
					startTime = stressIntensityReading->mTimeStamp;
				
				if (index == inObjects.size() - 1)
					endTime = stressIntensityReading->mTimeStamp;

				total += stressIntensityReading->mObject.mValue;
			}

			return std::make_unique<StressIntensityReadingSummary>(startTime, endTime, total / inObjects.size());
		}, error);

		if (!result)
			return false;

		result = mDataModel.registerType<EStressState>([](const std::vector<std::unique_ptr<rtti::Object>>& inObjects)
		{
			std::unordered_map<EStressState, int> stateCounts;
			TimeStamp startTime;
			TimeStamp endTime;
			for (int index = 0; index < inObjects.size(); ++index)
			{
				rtti::Object* object = inObjects[index].get();
				StressStateReading* stressStateReading = rtti_cast<StressStateReading>(object);
				assert(stressStateReading);

				if (index == 0)
					startTime = stressStateReading->mTimeStamp;

				if (index == inObjects.size() - 1)
					endTime = stressStateReading->mTimeStamp;

				stateCounts[stressStateReading->mObject]++;
			}

			EStressState maxStressState = EStressState::Unknown;
			int maxStressStateCount = 0;
			for (auto& kvp : stateCounts)
			{
				if (kvp.second > maxStressStateCount)
				{
					maxStressStateCount = kvp.second;
					maxStressState = kvp.first;
				}
			}

			return std::make_unique<StressStateReadingSummary>(startTime, endTime, maxStressState);
		}, error);
		
		if (!result)
			return false;

		return true;
	}
	

	void EmographyApp::updateGui()
	{
		// Stress intensity rendering
		ImGui::Begin("StressIntensity (average)");
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());

		{
			std::vector<float> values;

			std::vector<std::unique_ptr<rtti::Object>> rawValues;
			utility::ErrorState errorState;
			if (mDataModel.getLast<StressIntensityReading>(-1, 100, rawValues, errorState))			
			{
				values.resize(100 - rawValues.size());
				for (auto& rawValue : rawValues)
				{
					StressIntensityReading* stressIntensity = rtti_cast<StressIntensityReading>(rawValue.get());
					values.push_back(stressIntensity->mObject.mValue);
				}
			}

			ImGui::PlotLines("Raw", values.data(), values.size(), 0, nullptr, 1.0f, 100.0f, ImVec2(1000, 100));
		}
		
		for (int lod = 0; lod < 5; ++lod)
		{
			std::vector<float> values;

			utility::ErrorState errorState;
			std::vector<std::unique_ptr<rtti::Object>> summaries;
			if (mDataModel.getLast<StressIntensityReading>(lod, 100, summaries, errorState))			
			{
				values.resize(100 - summaries.size());
				for (auto& summary : summaries)
				{
					StressIntensityReadingSummary* stressIntensitySummary = rtti_cast<StressIntensityReadingSummary>(summary.get());
					values.push_back(stressIntensitySummary->mObject.mValue);
				}
			}

			ImGui::PlotLines(utility::stringFormat("LOD %d", lod + 1).c_str(), values.data(), values.size(), 0, nullptr, 1.0f, 100.0f, ImVec2(1000, 100));
		}

		ImGui::End();

		// Stress state rendering
		ImGui::Begin("StressState (most common)");

		{
			std::vector<float> values;

			utility::ErrorState errorState;
			std::vector<std::unique_ptr<rtti::Object>> rawValues;			
			if (mDataModel.getLast<StressStateReading>(-1, 100, rawValues, errorState))			
			{
				values.resize(100 - rawValues.size());
				for (auto& rawValue : rawValues)
				{
					StressStateReading* stressIntensity = rtti_cast<StressStateReading>(rawValue.get());
					values.push_back((float)stressIntensity->mObject);
				}
			}

			ImGui::PlotLines("Raw", values.data(), values.size(), 0, nullptr, -1.0f, 3.0f, ImVec2(1000, 100));
		}

		for (int lod = 0; lod < 5; ++lod)
		{
			std::vector<float> values;

			utility::ErrorState errorState;
			std::vector<std::unique_ptr<rtti::Object>> summaries;
			if (mDataModel.getLast<StressStateReading>(lod, 100, summaries, errorState))			
			{
				values.resize(100 - summaries.size());
				for (auto& summary : summaries)
				{
					StressStateReadingSummary* stressIntensitySummary = rtti_cast<StressStateReadingSummary>(summary.get());
					values.push_back((float)stressIntensitySummary->mObject);
				}
			}

			ImGui::PlotLines(utility::stringFormat("LOD %d", lod + 1).c_str(), values.data(), values.size(), 0, nullptr, -1.0f, 3.0f, ImVec2(1000, 100));
		}

		ImGui::End();
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
		}

		updateGui();
	}

	void EmographyApp::render()
	{
		mRenderService->destroyGLContextResources({ mRenderWindow });

		// Make render window active for drawing
		mRenderWindow->makeActive();

		// Clear target
		opengl::RenderTarget& render_target = mRenderWindow->getBackbuffer();
		render_target.setClearColor({ 0.0f, 0.0f, 1.0f, 1.0 });
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
 
