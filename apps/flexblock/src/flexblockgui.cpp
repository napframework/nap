// Local Includes
#include "flexblockgui.h"
#include "flexblockapp.h"
#include "flexblockcomponent.h"


// External Includes
#include <imgui/imgui.h>
#include <imguiutils.h>
#include <nap/core.h>
#include <utility/fileutils.h>
#include <parametergui.h>
#include <meshutils.h>
#include <sequence.h>
#include <sequenceplayercomponent.h>

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
		{
			showSequencesWindow(mShowTimeLine);

			if (mShowTimeLine)
				showTimeLine();
		}
			
	}
	

	void FlexblockGui::render()
	{
		mApp.getCore().getService<IMGuiService>()->draw();
	}


	void FlexblockGui::toggleVisibility()
	{
		mHide = !mHide;
	}

	void FlexblockGui::showTimeLine()
	{
		// Fetch resource manager to get access to all loaded resources
		ResourceManager* resourceManager = mApp.getCore().getResourceManager();

		ImGui::Begin("Sequence Timeline");

		/*
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		glm::vec2 top_left = glm::vec2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
		const auto max = ImGui::GetWindowContentRegionMax();
		const auto min = ImGui::GetWindowContentRegionMin();

		const auto size = ImVec2(max.x - min.x, max.y - min.y);

		timeline::SequencePlayerComponentInstance& sequencePlayer = mApp.GetBlockEntity()->getComponent<timeline::SequencePlayerComponentInstance>();

		const auto & elements = sequencePlayer.getElements();
		for (int i = 0; i < elements.size(); i++)
		{
			float width = size.x * (elements[i]->mDuration / sequencePlayer.getDuration());
			float start_x = size.x * (elements[i]->getStartTime() / sequencePlayer.getDuration());

			draw_list->AddLine(ImVec2(top_left.x + start_x, top_left.y), ImVec2(top_left.x + start_x, top_left.y + size.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));
			draw_list->AddLine(ImVec2(top_left.x + start_x, top_left.y + size.y), ImVec2(top_left.x + start_x + width, top_left.y + size.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));

			//draw_list->AddText(ImVec2(top_left.x + start_x, top_left.y + size.y), ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 1.0f, 0.0f, 1.0f)), elements[i]->mID.c_str());
		}

		//
		draw_list->AddLine(ImVec2(top_left.x + size.x, top_left.y), ImVec2(top_left.x + size.x, top_left.y + size.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));

		float pos = ( sequencePlayer.getCurrentTime() / sequencePlayer.getDuration() ) * size.x;
		draw_list->AddLine(ImVec2(top_left.x + pos, top_left.y), ImVec2(top_left.x + pos, top_left.y + size.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)));

		*/
		ImGui::End();
	}

	void FlexblockGui::showSequencesWindow(bool &showTimeLine)
	{
		// Fetch resource manager to get access to all loaded resources
		ResourceManager* resourceManager = mApp.getCore().getResourceManager();

		ImGui::Begin("Sequence player");
		ImGui::Spacing();

		timeline::SequencePlayerComponentInstance& sequencePlayer = mApp.GetBlockEntity()->getComponent<timeline::SequencePlayerComponentInstance>();
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

		ImGui::SameLine();

		bool isLooping = sequencePlayer.getIsLooping();
		if (ImGui::Checkbox("Loop", &isLooping))
		{
			sequencePlayer.setIsLooping(isLooping);
		}

		ImGui::Spacing();

		if (ImGui::TreeNode("Sequences"))
		{
			const auto& sequences = sequencePlayer.getSequences();

			for (const auto& sequence : sequences)
			{
				ImVec4 color = ImVec4(1, 1, 1, 1);

				if (sequencePlayer.getCurrentSequence() == sequence)
				{
					color = ImVec4(1, 0, 0, 1);
				}

				ImGui::PushStyleColor(1, color);
				if(ImGui::Button(sequence->mID.c_str()))
				{
					sequencePlayer.skipToSequence(sequence);
				}
				ImGui::PopStyleColor();
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

			ImGui::Text(std::string("Current Sequence Element = " + sequencePlayer.getCurrentSequence()->getCurrentElement()->mID).c_str());
			ImGui::Spacing();
			ImGui::Text("Progress");

			mScrub = (float)(sequencePlayer.getCurrentTime() / sequencePlayer.getDuration());
			ImGui::ProgressBar(mScrub);
			auto progressBarSize = ImGui::GetItemRectSize();

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