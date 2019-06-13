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
#include <iomanip>

namespace nap
{
	/**
	 * Imgui statics
	 */
	static bool showInfo = false;
	static bool showPresetWindow = false;
	static bool showSequences = false;
	static float lengthPerSecond = 60.0f;
	static float child_width = 1000.0f;
	static bool wasClicked = false;

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
				ImGui::MenuItem("Timeline", NULL, &mShowTimeLine);

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (showPresetWindow)
			mParameterGUI->show(mParameterService.hasRootGroup() ? &mParameterService.getRootGroup() : nullptr);

		if (showInfo)
			showInfoWindow();

		if (mShowTimeLine)
			showTimeLineWindow();
			
		if (mShowSequenceList)
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

	void FlexblockGui::showTimeLineWindow()
	{
		timeline::SequencePlayerComponentInstance& sequencePlayer = mApp.GetBlockEntity()->getComponent<timeline::SequencePlayerComponentInstance>();

		ImGui::SetNextWindowContentSize(ImVec2(child_width, 300.0f));
		ImGui::Begin("Timeline", 0, ImGuiWindowFlags_HorizontalScrollbar);

		float cursorPosX = ImGui::GetCursorPosX();
		ImGui::SetCursorPos(ImVec2(cursorPosX + ImGui::GetScrollX(), ImGui::GetCursorPosY()));

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

		ImGui::SameLine();
		float speed = sequencePlayer.getSpeed();
		ImGui::PushItemWidth(50.0f);
		ImGui::DragFloat("Speed", &speed, 0.05f, -5.0f, 5.0f);
		ImGui::PopItemWidth();
		sequencePlayer.setSpeed(speed);

		ImGui::SameLine();
		ImGui::Checkbox("Show Sequence List", &mShowSequenceList);

		ImGui::SameLine();
		ImGui::PushItemWidth(100.0f);
		ImGui::DragFloat("Resolution", &lengthPerSecond, 0.5f, 10.0f, 200.0f);
		ImGui::PopItemWidth();
		child_width = sequencePlayer.getDuration() * lengthPerSecond;

		lengthPerSecond += ImGui::GetIO().MouseWheel;
		lengthPerSecond = math::clamp<float>(lengthPerSecond, 10.0f, 200.0f);

		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::SetCursorPos(ImVec2(cursorPosX, ImGui::GetCursorPosY()));

		ImGui::BeginChild("", ImVec2(child_width, 275.0f), false, ImGuiWindowFlags_NoMove);
		{
			const auto child_size = ImVec2(child_width, 250.0f);

			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			//
			ImVec2 top_left = ImGui::GetCursorScreenPos();

			ImVec2 bottom_right_pos = ImVec2(top_left.x + child_size.x, top_left.y + child_size.y );

			draw_list->AddRectFilled(top_left, bottom_right_pos,
				ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.25f)));



			const auto & sequences = sequencePlayer.getSequences();
			for (int i = 0; i < sequences.size(); i++)
			{
				float width = child_size.x * (sequences[i]->getDuration() / sequencePlayer.getDuration());
				float start_x = child_size.x * (sequences[i]->getStartTime() / sequencePlayer.getDuration());

				draw_list->AddLine(ImVec2(top_left.x + start_x, top_left.y), ImVec2(top_left.x + start_x, top_left.y + child_size.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));
				draw_list->AddLine(ImVec2(top_left.x + start_x, bottom_right_pos.y), ImVec2(top_left.x + start_x + width, bottom_right_pos.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));

				int y_text_offset = 0;
				for (const auto* element : sequences[i]->mElements)
				{
					float p = (element->getStartTime() - sequences[i]->getStartTime()) / sequences[i]->getDuration();

					draw_list->AddLine(
						ImVec2(top_left.x + start_x + width * p, top_left.y + child_size.y * 0.5f),
						ImVec2(top_left.x + start_x + width * p, bottom_right_pos.y),
						ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 1.0f, 0.0f, 1.0f)));

					draw_list->AddText(
						ImVec2(top_left.x + start_x + width * p + 5, top_left.y + child_size.y * 0.5f + y_text_offset),
						ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 1.0f, 0.0f, 1.0f)),
						element->getID().c_str());

					y_text_offset += 20;
					if (y_text_offset > child_size.y * 0.5f - 20)
					{
						y_text_offset = 0;
					}
				}

				draw_list->AddText(
					ImVec2(top_left.x + start_x + 5, top_left.y + 15),
					ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)),
					sequences[i]->getID().c_str());
			}

			//
			draw_list->AddLine(
				ImVec2(top_left.x + child_size.x, top_left.y),
				ImVec2(top_left.x + child_size.x, bottom_right_pos.y),
				ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));

			float pos = (sequencePlayer.getCurrentTime() / sequencePlayer.getDuration()) * child_size.x;
			draw_list->AddLine(ImVec2(top_left.x + pos, top_left.y), ImVec2(top_left.x + pos, bottom_right_pos.y),
				ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)));

			draw_list->AddText(
				ImVec2(top_left.x + pos + 5, top_left.y),
				ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)),
				convertToString(sequencePlayer.getCurrentTime(), 2).c_str());

			if (ImGui::IsMouseHoveringRect(top_left, bottom_right_pos))
			{
				if (ImGui::IsMouseClicked(0))
				{
					wasClicked = true;
					ImVec2 mousePos = ImGui::GetMousePos();
					mousePos = ImVec2(mousePos.x - top_left.x, mousePos.y - top_left.y);
					float pos = mousePos.x / child_size.x;
					sequencePlayer.setTime(pos * sequencePlayer.getDuration());
					//printf("mouse pos %f\n", mousePos.x);
				}
				else if (ImGui::IsMouseDragging() && wasClicked)
				{
					ImVec2 mousePos = ImGui::GetMousePos();
					mousePos = ImVec2(mousePos.x - top_left.x, mousePos.y - top_left.y);
					float pos = mousePos.x / child_size.x;
					sequencePlayer.setTime(pos * sequencePlayer.getDuration());
					//printf("mouse pos %f\n", mousePos.x);
				}
			}

			if (!ImGui::IsMouseDown(0))
				wasClicked = false;

			ImGui::EndChild();

			ImGui::Spacing();
			ImGui::Spacing();
		}

		ImGui::End();
	}

	template<typename T1>
	std::string FlexblockGui::convertToString(T1 number, int precision)
	{
		std::ostringstream streamObj;
		streamObj << std::fixed;
		streamObj << std::setprecision(precision) << number;
		return streamObj.str();
	}

	void FlexblockGui::showSequencesWindow()
	{
		// Fetch resource manager to get access to all loaded resources
		ResourceManager* resourceManager = mApp.getCore().getResourceManager();

		ImGui::Begin("Sequence player");
		ImGui::Spacing();

		timeline::SequencePlayerComponentInstance& sequencePlayer = mApp.GetBlockEntity()->getComponent<timeline::SequencePlayerComponentInstance>();


		ImGui::Spacing();

		if (ImGui::TreeNode("Sequences"))
		{
			const auto& sequences = sequencePlayer.getSequences();

			for (const auto* sequence : sequences)
			{
				ImVec4 color = ImVec4(1, 1, 1, 1);

				bool isSequenceBeingPlayed = sequencePlayer.getCurrentSequence() == sequence;
				if (isSequenceBeingPlayed)
				{
					color = ImVec4(1, 0, 0, 1);
				}

				ImGui::PushStyleColor(0, color);
				if(ImGui::SmallButton(sequence->getID().c_str()))
				{
					sequencePlayer.skipToSequence(sequence);
				}
				ImGui::PopStyleColor();

				if (ImGui::TreeNode(std::string(sequence->getID() + " Elements").c_str()))
				{
					const auto& elements = sequence->mElements;

					for(const auto* element : elements)
					{
						bool isElementBeingPlayed =
							sequence->getCurrentElement() == element;

						if( isElementBeingPlayed && isSequenceBeingPlayed )
							ImGui::PushStyleColor(0, color);

						if (ImGui::SmallButton(element->getID().c_str()))
						{
							sequencePlayer.skipToSequence(sequence);
							sequencePlayer.setTime(element->getStartTime());
						}

						if (isElementBeingPlayed && isSequenceBeingPlayed)
							ImGui::PopStyleColor();
					}

					ImGui::TreePop();
				}

			}
			ImGui::TreePop();
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