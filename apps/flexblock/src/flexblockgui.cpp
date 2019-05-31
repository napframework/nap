// Local Includes
#include "flexblockgui.h"
#include "flexblockapp.h"
#include "flexblockcomponent.h"
#include "flexblocksequence.h"
#include "flexblocksequenceplayercomponent.h"

// External Includes
#include <imgui/imgui.h>
#include <imguiutils.h>
#include <nap/core.h>
#include <utility/fileutils.h>
#include <parametergui.h>
#include <meshutils.h>

namespace nap
{
	/**
	 * Imgui statics
	 */
	static bool showInfo = false;
	static bool showPresetWindow = false;
	static bool showSequences = false;

	FlexblockGui::FlexblockGui(FlexblockApp& app) : 
		mApp(app),
		mParameterService(*app.getCore().getService<ParameterService>())
	{
	}


	FlexblockGui::~FlexblockGui()
	{
	}


	void FlexblockGui::init()
	{
		// Create parameter gui
		mParameterGUI = std::make_unique<ParameterGUI>(mParameterService);

		// Fetch resource manager to get access to all loaded resources
		ResourceManager* resourceManager = mApp.getCore().getResourceManager();
		
		//
		initParameters();

		mParameterService.fileLoaded.connect(
			[&]() -> void { initParameters(); }
		);
	}

	void FlexblockGui::initParameters()
	{
		// Fetch resource manager to get access to all loaded resources
		ResourceManager* resourceManager = mApp.getCore().getResourceManager();

		//
		FlexBlockComponentInstance& flexblockComponent = mApp.GetBlockEntity()->getComponent<FlexBlockComponentInstance>();
		for (int i = 0; i < 8; i++)
		{
			std::string id = "Input " + std::to_string(i + 1);
			ObjectPtr<ParameterFloat> parameter = resourceManager->findObject<ParameterFloat>(id);
			
			assert(parameter != nullptr);

			parameter->setValue(0.0f);
			parameter->valueChanged.connect([this, i](float newValue)
			{
				updateInput(i, newValue);
			});
		}
	}

	void FlexblockGui::updateInput(int index, float value)
	{
		ObjectPtr<EntityInstance> blockEntity = mApp.GetBlockEntity();
		FlexBlockComponentInstance& flexblockComponent = blockEntity->getComponent<FlexBlockComponentInstance>();
		flexblockComponent.SetMotorInput(index, value);
	}

	void FlexblockGui::update()
	{
		if (mHide)
			return;

		// Menu
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Display"))
			{
				ImGui::MenuItem("Parameters", NULL, &showPresetWindow);
				ImGui::MenuItem("Information", NULL, &showInfo);
				ImGui::MenuItem("Sequences", NULL, &showSequences);

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (showPresetWindow)
			mParameterGUI->show(mParameterService.hasRootGroup() ? &mParameterService.getRootGroup() : nullptr);

		if (showInfo)
			showInfoWindow();

		if (showSequences)
			showSequencesWindow();
	}
	

	void FlexblockGui::render()
	{
		mApp.getCore().getService<IMGuiService>()->draw();
	}


	void FlexblockGui::toggleVisibility()
	{
		mHide = !mHide;
	}

	void FlexblockGui::showSequencesWindow()
	{
		// Fetch resource manager to get access to all loaded resources
		ResourceManager* resourceManager = mApp.getCore().getResourceManager();

		ImGui::Begin("Sequence player");
		ImGui::Spacing();

		FlexBlockSequencePlayerComponentInstance& sequencePlayer = mApp.GetBlockEntity()->getComponent<FlexBlockSequencePlayerComponentInstance>();
		if (ImGui::Button("Stop"))
		{
			sequencePlayer.stop();
		}

		if (sequencePlayer.getIsLoaded())
		{
			ImGui::SameLine();
			if (!sequencePlayer.getIsPaused() &&
				sequencePlayer.getIsPlaying())
			{
				if (ImGui::Button("Pause"))
				{
					sequencePlayer.pause();
				}

			}
			else
			{
				if (sequencePlayer.getIsFinished())
				{
					sequencePlayer.setTime(0.0);
				}

				if (ImGui::Button("Play"))
				{
					sequencePlayer.play();
				}
			}
		}

		ImGui::Spacing();

		if (ImGui::TreeNode("Sequences"))
		{
			auto& sequences = resourceManager->getObjects<FlexBlockSequence>();

			for (auto& sequence : sequences)
			{
				ImGui::Text(sequence->mID.c_str());
				ImGui::SameLine();
				if (ImGui::SmallButton("load"))
				{
					sequencePlayer.load(sequence.get());
					sequencePlayer.play();
				}
			}
			ImGui::TreePop();
		}

		ImGui::Spacing();
		ImGui::Spacing();

		if (sequencePlayer.getIsPlaying())
		{
			ImGui::Text(("Playing sequence [" 
				+ sequencePlayer.getCurrentSequence()->mID 
				+ "]").c_str());
			RGBColorFloat text_color = mTextColor.convert<RGBColorFloat>();
			
			ImGui::SameLine();
			ImGui::TextColored(text_color, "%.2f seconds", sequencePlayer.getCurrentTime() );
		
			ImGui::Spacing();
			ImGui::Text(("Current Element = " + sequencePlayer.getCurrentElement()->mID).c_str());
			ImGui::Spacing();
			ImGui::Text("Progress");

			mScrub = (float)(sequencePlayer.getCurrentTime() / sequencePlayer.getDuration());
			ImGui::ProgressBar(mScrub);
			auto progressBarSize = ImGui::GetItemRectSize();

			ImGui::NewLine();

			ImGui::Text("Elements");
			for (const auto& element : sequencePlayer.getElements())
			{
				if (ImGui::Button(element->mID.c_str(), ImVec2(progressBarSize.x * element->mDuration / sequencePlayer.getDuration(), progressBarSize.y)))
				{
					sequencePlayer.setTime(element->getStartTime());
				}
				ImGui::SameLine();
			}

			ImGui::NewLine();
			ImGui::Spacing();
			ImGui::Text("Scrub");
			if (ImGui::SliderFloat("", &mScrub, 0.0f, 1.0f))
			{
				sequencePlayer.setTime(mScrub * sequencePlayer.getDuration());
			}
		}

		ImGui::End();
	}

	void FlexblockGui::showInfoWindow()
	{
		// Color used for highlights
		mApp.getCore().getFramerate();

		ImGui::Begin("Information");
		ImGui::Spacing();
		getCurrentDateTime(mDateTime);
		ImGui::Text(mDateTime.toString().c_str());
		RGBColorFloat text_color = mTextColor.convert<RGBColorFloat>();
		ImGui::TextColored(text_color, "%.3f ms/frame (%.1f FPS)", 1000.0f / mApp.getCore().getFramerate(), mApp.getCore().getFramerate());
		ImGui::End();
	}

}