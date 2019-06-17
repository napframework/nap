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
	static float child_height = 500.0f;
	static bool wasClicked = false;
	static bool followPlayer = false;
	static int curveResolution = 75;

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
		mSequencePlayer = mApp.GetBlockEntity()->findComponent<timeline::SequencePlayerComponentInstance>();
		
		//
		mFlexBlock = mApp.GetBlockEntity()->findComponent<FlexBlockComponentInstance>();

		//
		initParameters();

		//
		initOscInputs();

		mParameterService.fileLoaded.connect(
			[&]() -> void { 
			initParameters(); 
			initOscInputs();
		});
	}

	void FlexblockGui::initOscInputs()
	{
		mOscInputs.clear();
		for (int i = 0; i < 8; i++)
		{
			//
			OSCInputComponentInstance* oscInput = mApp.GetBlockEntity()->findComponentByID<OSCInputComponentInstance>("OSCInput " + std::to_string(i + 1));
			mOscInputs.emplace_back(oscInput);

			//
			oscInput->messageReceived.connect([this, i](const OSCEvent& message)-> void {
				float value = message.getArgument(0)->asFloat();
				mParameters[i]->setValue(value);
			});
		}
	}

	void FlexblockGui::initParameters()
	{
		// Fetch resource manager to get access to all loaded resources
		ResourceManager* resourceManager = mApp.getCore().getResourceManager();

		//
		mParameters.clear();

		//
		mSequencePlayer = &mApp.GetBlockEntity()->getComponent<timeline::SequencePlayerComponentInstance>();
		mFlexBlock = &mApp.GetBlockEntity()->getComponent<FlexBlockComponentInstance>();
		for (int i = 0; i < 8; i++)
		{
			std::string id = "Input " + std::to_string(i + 1);
			ObjectPtr<ParameterFloat> parameter = resourceManager->findObject<ParameterFloat>(id);
			
			assert(parameter != nullptr);

			mParameters.emplace_back(parameter.get());
			parameter->setValue(0.0f);
			parameter->valueChanged.connect([this, i](float newValue)
			{
				updateInput(i, newValue);
			});
		}
	}

	void FlexblockGui::updateInput(int index, float value)
	{
		mFlexBlock->SetMotorInput(index, value);
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
				ImGui::MenuItem("Timeline", NULL, &mShowTimeLine);
				ImGui::MenuItem("Sequences", NULL, &mShowSequenceList);

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
		// set next window content size to timeline ( child ) width to make scroll bar fit
		ImGui::SetNextWindowContentSize(ImVec2(child_width + 100.0f, child_height + 150.0f ));

		// begin the window
		ImGui::Begin("Timeline", 0, ImGuiWindowFlags_HorizontalScrollbar );

		float windowWidth = ImGui::GetWindowWidth();

		// make sure the top elements above the timeline scroll together with the scrollbar so only timeline moves
		float cursorPosX = ImGui::GetCursorPosX();
		ImGui::SetCursorPos(ImVec2(cursorPosX + ImGui::GetScrollX(), ImGui::GetCursorPosY()));

		// stop button
		if (ImGui::Button("Stop"))
		{
			mSequencePlayer->stop();
		}

		// draw sequence player controls
		if (mSequencePlayer->getIsLoaded())
		{
			ImGui::SameLine();
			if (!mSequencePlayer->getIsPaused() &&
				mSequencePlayer->getIsPlaying())
			{
				if (ImGui::Button("Pause"))
				{
					mSequencePlayer->pause();
				}
			}
			else
			{
				if (mSequencePlayer->getIsFinished())
				{
					mSequencePlayer->setTime(0.0);
				}

				if (ImGui::Button("Play"))
				{
					mSequencePlayer->play();
				}
			}
		}

		ImGui::SameLine();

		// loop
		bool isLooping = mSequencePlayer->getIsLooping();
		if (ImGui::Checkbox("Loop", &isLooping))
		{
			mSequencePlayer->setIsLooping(isLooping);
		}

		// follow player position
		ImGui::SameLine();
		ImGui::Checkbox("Follow", &followPlayer);
		if (followPlayer)
		{
			ImGui::SetScrollX((mSequencePlayer->getCurrentTime() / mSequencePlayer->getDuration()) * child_width);
		}

		// speed
		ImGui::SameLine();
		float speed = mSequencePlayer->getSpeed();
		ImGui::PushItemWidth(50.0f);
		ImGui::DragFloat("Speed", &speed, 0.05f, -5.0f, 5.0f);
		ImGui::PopItemWidth();
		mSequencePlayer->setSpeed(speed);

		// zoom of timeline
		ImGui::SameLine();
		ImGui::PushItemWidth(100.0f);
		ImGui::DragFloat("Zoom", &child_height, 1, 350.0f, 1500.0f );
		ImGui::PopItemWidth();

		// curves resolution
		ImGui::SameLine();
		ImGui::PushItemWidth(100.0f);
		ImGui::DragInt("Curve Res.", &curveResolution, 1, 25, 200);
		ImGui::PopItemWidth();

		// handle scroll wheel input
		lengthPerSecond += ImGui::GetIO().MouseWheel;
		lengthPerSecond = math::clamp<float>(lengthPerSecond, 10.0f, 200.0f);
		child_width = mSequencePlayer->getDuration() * lengthPerSecond;

		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::SetCursorPos(ImVec2(cursorPosX, ImGui::GetCursorPosY()));

		// 
		float scroll_x = ImGui::GetScrollX();

		// create list of point lists
		std::vector<std::vector<ImVec2>> curvePoints(8);

		// begin timeline child
		ImGui::BeginChild("", ImVec2(child_width + 20, child_height), false, ImGuiWindowFlags_NoMove);
		{
			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			//
			const ImVec2 child_size = ImVec2(child_width, child_height - 100.0f);

			// start top left with a little bit of margin
			ImVec2 top_left = ImGui::GetCursorScreenPos();
			top_left.x += 20;
			top_left.y += 20;
			ImVec2 bottom_right_pos = ImVec2(top_left.x + child_size.x, top_left.y + child_size.y );

			// timeline background
			draw_list->AddRectFilled(top_left, bottom_right_pos,
				ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.25f)), 1.0f);

			// motors backgrounds
			for (int l = 0; l < 8; l++)
			{
				draw_list->AddRect(	ImVec2( top_left.x, math::lerp<float>( top_left.y, bottom_right_pos.y, (float) l / 8.0f) ),
									ImVec2( bottom_right_pos.x, math::lerp<float>(top_left.y, bottom_right_pos.y, (float)(l+1) / 8.0f)),
									ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.5f)), 1.0f);
			}

			// right border
			draw_list->AddLine(
				ImVec2(top_left.x + child_size.x, top_left.y),
				ImVec2(top_left.x + child_size.x, bottom_right_pos.y),
				ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));

			// needed to avoid elements texts from overlapping later on
			int y_text_offset = 0;

			// iterate through sequences 
			const auto & sequences = mSequencePlayer->getSequences();
			for (int i = 0; i < sequences.size(); i++)
			{
				float width = child_size.x * (sequences[i]->getDuration() / mSequencePlayer->getDuration());
				float start_x = child_size.x * (sequences[i]->getStartTime() / mSequencePlayer->getDuration());

				// draw sequence line
				draw_list->AddLine(
					ImVec2(top_left.x + start_x, top_left.y), 
					ImVec2(top_left.x + start_x, top_left.y + child_size.y + 75), 
					ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));

				// draw sequence text
				draw_list->AddText(
					ImVec2(top_left.x + start_x + 5, top_left.y + child_size.y + 60),
					ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)),
					sequences[i]->getID().c_str());

				// draw element lines and positions
				for (const auto* element : sequences[i]->mElements)
				{
					float element_pos = (element->getStartTime() - sequences[i]->getStartTime()) / sequences[i]->getDuration();

					// the bottom line
					draw_list->AddLine(
						ImVec2(top_left.x + start_x + width * element_pos, bottom_right_pos.y),
						ImVec2(top_left.x + start_x + width * element_pos, bottom_right_pos.y + 50),
						ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 1.0f, 0.0f, 1.0f)));

					// line in timeline
					draw_list->AddLine(
						ImVec2(top_left.x + start_x + width * element_pos, top_left.y),
						ImVec2(top_left.x + start_x + width * element_pos, bottom_right_pos.y + 50),
						ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 1.0f, 0.0f, 0.5f)));

					// the text
					draw_list->AddText(
						ImVec2(top_left.x + start_x + width * element_pos + 5, bottom_right_pos.y + y_text_offset),
						ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 1.0f, 0.0f, 1.0f)),
						element->getID().c_str());
						

					// set a height offset of the next text so they don't overlap to much
					y_text_offset += 20;
					if (y_text_offset > 50 - 10)
					{
						y_text_offset = 0;
					}

					// draw motor inputs 
					bool showMotorInputs = true;
					if (showMotorInputs)
					{
						// create parameters that we evaluate
						std::vector<std::unique_ptr<ParameterFloat>> parametersPts;
						std::vector<Parameter*> parameters;
						for (int p = 0; p < 8; p++)
						{
							parametersPts.emplace_back(std::make_unique<ParameterFloat>());
							parameters.emplace_back(parametersPts.back().get());
						}

						// zoom in on the part that is shown in the window
						const int steps = curveResolution;
						float part =  windowWidth / child_width;
						float part_start = math::clamp<float>(scroll_x - 30, 0, child_width ) / child_width;

						// start evaluating
						for (int p = 0; p < steps; p++)
						{
							//
							mSequencePlayer->evaluate(( ( mSequencePlayer->getDuration() * part ) / (float) steps) * (float)p + (mSequencePlayer->getDuration() * part_start), parameters);

							for (int l = 0; l < 8; l++)
							{
								float y_part = (child_size.y / 8.0f);
								float y_start = y_part * l;

								curvePoints[l].emplace_back(ImVec2(
									part_start * child_width + top_left.x + child_size.x * part * (p * (1.0f / (float) steps)),
									bottom_right_pos.y - y_start - y_part * static_cast<ParameterFloat*>(parameters[l])->mValue));
							}
						}
					}
				}
			}

							// draw the polylines and text
				for (int l = 0; l < 8; l++)
				{
					draw_list->AddPolyline(
						&*curvePoints[l].begin(),
						curvePoints[l].size(),
						ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 0.0f, 1.0f)),
						false,
						2.0f,
						true);

					draw_list->AddText(
						ImVec2(top_left.x - 15, top_left.y + (child_size.y / 8) * l + 4),
						ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)),
						std::to_string(8 - l).c_str());
				}

			// draw player position 
			float player_pos = (mSequencePlayer->getCurrentTime() / mSequencePlayer->getDuration()) * child_size.x;
			draw_list->AddLine(ImVec2(top_left.x + player_pos, top_left.y), ImVec2(top_left.x + player_pos, bottom_right_pos.y),
				ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)));

			// time in seconds
			draw_list->AddText(
				ImVec2(top_left.x + player_pos + 5, top_left.y - 20),
				ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)),
				convertToString(mSequencePlayer->getCurrentTime(), 2).c_str());

			// handle mouse input
			if (ImGui::IsMouseHoveringRect(top_left, bottom_right_pos))
			{
				// is clicked inside timeline ? jump to position
				if (ImGui::IsMouseClicked(0))
				{
					wasClicked = true;
					ImVec2 mousePos = ImGui::GetMousePos();
					mousePos = ImVec2(mousePos.x - top_left.x, mousePos.y - top_left.y);
					float pos = mousePos.x / child_size.x;
					mSequencePlayer->setTime(pos * mSequencePlayer->getDuration());
				}
				// handle drag in timeline
				else if (ImGui::IsMouseDragging() && wasClicked)
				{
					ImVec2 mousePos = ImGui::GetMousePos();
					mousePos = ImVec2(mousePos.x - top_left.x, mousePos.y - top_left.y);
					float pos = mousePos.x / child_size.x;
					mSequencePlayer->setTime(pos * mSequencePlayer->getDuration());
				}
			}

			// release mouse
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

		ImGui::Spacing();

		if (ImGui::TreeNode("Sequences"))
		{
			const auto& sequences = mSequencePlayer->getSequences();

			for (const auto* sequence : sequences)
			{
				ImVec4 color = ImVec4(1, 1, 1, 1);

				bool isSequenceBeingPlayed = mSequencePlayer->getCurrentSequence() == sequence;
				if (isSequenceBeingPlayed)
				{
					color = ImVec4(1, 0, 0, 1);
				}

				ImGui::PushStyleColor(0, color);
				if(ImGui::SmallButton(sequence->getID().c_str()))
				{
					mSequencePlayer->skipToSequence(sequence);
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
							mSequencePlayer->skipToSequence(sequence);
							mSequencePlayer->setTime(element->getStartTime());
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