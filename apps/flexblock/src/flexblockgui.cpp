// Local Includes
#include "flexblockgui.h"
#include "flexblockapp.h"
#include "flexblockcomponent.h"
#include "flexblocksequence.h"
#include "flexblocksequencetransition.h"
#include "sequencepause.h"

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
#include <sequencetransition.h>
#include <math.h>       /* modf */

//#include <ctime.h>

namespace nap
{
	// 200, 105, 105
	ImU32 colorRed = ImGui::ColorConvertFloat4ToU32(ImVec4(200.0f / 255.0f, 105.0f / 255.0f, 105.0f / 255.0f, 1.0f));

	// 17, 20, 38
	ImU32 colorBlack = ImGui::ColorConvertFloat4ToU32(ImVec4(17.0f / 255.0f, 20.0f / 255.0f, 38.0f / 255.0f, 1.0f));

	// 139, 140, 160
	ImU32 colorWhite = ImGui::ColorConvertFloat4ToU32(ImVec4(139.0f / 255.0f, 140.0f / 255.0f, 160.0f / 255.0f, 1.0f));

	// 93, 94, 115
	ImU32 colorLightGrey = ImGui::ColorConvertFloat4ToU32(ImVec4(93.0f / 255.0f, 94.0f / 255.0f, 115.0f / 255.0f, 1.0f));

	// 82, 84, 106
	ImU32 colorDarkGrey = ImGui::ColorConvertFloat4ToU32(ImVec4(82.0f / 255.0f, 84.0f / 255.0f, 106.0f / 255.0f, 1.0f));

	ImU32 colorGreen = ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.5f, 0.0f, 1.0f));

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

		// Controller
		mMotorController = resourceManager->findObject<MACController>("MACController");

		//
		initOscOutput();

		//
		initParameters();

		//
		initOscInputs();

		mParameterService.fileLoaded.connect(
			[&]() -> void { 
			initOscOutput();
			initParameters();
			initOscInputs();
		});
	}


	void FlexblockGui::initOscOutput()
	{
		mOscSender = nullptr;
		ObjectPtr<nap::rtti::Object> oscOutputObjectPtr = mApp.mResourceManager->findObject("OSCOutput");
		if (oscOutputObjectPtr != nullptr)
		{
			mOscSender = static_cast<OSCSender*>(oscOutputObjectPtr.get());
		}
	}


	void FlexblockGui::initOscInputs()
	{
		mOscInputs.clear();
		
		//
		OSCInputComponentInstance* oscInput = mApp.GetBlockEntity()->findComponentByID<OSCInputComponentInstance>("OSCMotorInputs");
			
		if (oscInput != nullptr)
		{
			mOscInputs.emplace_back(oscInput);

			//
			oscInput->messageReceived.connect([this](const OSCEvent& message)-> void
			{
				try
				{
					std::string adress = message.getAddress();
					std::vector<std::string> adressParts;
					utility::splitString(adress, '/', adressParts);

					if (adressParts.size() == 4)
					{
						if (adressParts[1] == "flexblock" &&
							adressParts[2] == "motor")
						{
							int parameter = std::stoi(adressParts[3]) - 1;
							if (parameter >= 0 && parameter < 8)
							{
								float value = message.getArgument(0)->asFloat();
								mParameters[parameter]->setValue(value);
							}
						}
					}else if(adressParts.size() == 3)
					{
						if (adressParts[1] == "flexblock" &&
							adressParts[2] == "slack")
						{
							int parameter = 8;
							float value = message.getArgument(0)->asFloat();
							mParameters[parameter]->setValue(value);
						}
					}
				}
				catch (std::exception& e)
				{
					printf(("OSC Error : " + std::string(e.what()) + "\n").c_str());
				}
			});
		}
	}


	void FlexblockGui::initParameters()
	{
		// Fetch resource manager to get access to all loaded resources
		ResourceManager* resourceManager = mApp.getCore().getResourceManager();

		//
		mParameters.clear();

		// motor parameters
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

				if (mOscSender != nullptr)
				{
					OSCEvent oscMessage("/flexblock/motor/" + std::to_string(i+1));
					oscMessage.addValue<float>(newValue);
					mOscSender->send(oscMessage);
				}
			});

			if (mOscSender != nullptr)
			{
				OSCEvent oscMessage("/flexblock/motor/" + std::to_string(i + 1));
				oscMessage.addValue<float>(0.0f);
				mOscSender->send(oscMessage);
			}
		}

		// slack parameter
		ObjectPtr<ParameterFloat> slackParameter = resourceManager->findObject<ParameterFloat>("Slack");

		assert(slackParameter != nullptr);

		mParameters.emplace_back(slackParameter.get());
		slackParameter->setValue(0.0f);

		slackParameter->valueChanged.connect([this](float newValue)
		{
			mFlexBlock->setSlack(newValue);

			if (mOscSender != nullptr)
			{
				OSCEvent oscMessage("/flexblock/slack");
				oscMessage.addValue<float>(newValue);
				mOscSender->send(oscMessage);
			}
		});

		if (mOscSender != nullptr)
		{
			OSCEvent oscMessage("/flexblock/slack");
			oscMessage.addValue<float>(0.5f);
			mOscSender->send(oscMessage);
		}

		// overrides
		for (int i = 0; i < 8; i++)
		{
			std::string id = "Override " + std::to_string(i + 1);
			ObjectPtr<ParameterFloat> parameter = resourceManager->findObject<ParameterFloat>(id);

			assert(parameter != nullptr);

			mParameters.emplace_back(parameter.get());
			parameter->setValue(0.0f);
			parameter->valueChanged.connect([this, i](float newValue)
			{
				updateOverride(i, newValue);

				if (mOscSender != nullptr)
				{
					OSCEvent oscMessage("/flexblock/override/" + std::to_string(i + 1));
					oscMessage.addValue<float>(newValue);
					mOscSender->send(oscMessage);
				}
			});

			if (mOscSender != nullptr)
			{
				OSCEvent oscMessage("/flexblock/override/" + std::to_string(i + 1));
				oscMessage.addValue<float>(0.0f);
				mOscSender->send(oscMessage);
			}
		}

		// sinus frequency
		ObjectPtr<ParameterFloat> sinusFrequencyParameter = resourceManager->findObject<ParameterFloat>("Sinus Frequency");

		assert(sinusFrequencyParameter != nullptr);

		mParameters.emplace_back(sinusFrequencyParameter.get());
		sinusFrequencyParameter->setValue(0.0f);

		sinusFrequencyParameter->valueChanged.connect([this](float newValue)
		{
			mFlexBlock->setSinusFrequency(newValue);

			if (mOscSender != nullptr)
			{
				OSCEvent oscMessage("/flexblock/sinus/frequency");
				oscMessage.addValue<float>(newValue);
				mOscSender->send(oscMessage);
			}
		});

		if (mOscSender != nullptr)
		{
			OSCEvent oscMessage("/flexblock/sinus/frequency");
			oscMessage.addValue<float>(0.5f);
			mOscSender->send(oscMessage);
		}

		// sinus amplitude
		ObjectPtr<ParameterFloat> sinusAmplitudeParameter = resourceManager->findObject<ParameterFloat>("Sinus Amplitude");

		assert(sinusAmplitudeParameter != nullptr);

		mParameters.emplace_back(sinusAmplitudeParameter.get());
		sinusAmplitudeParameter->setValue(0.0f);

		sinusAmplitudeParameter->valueChanged.connect([this](float newValue)
		{
			mFlexBlock->setSinusAmplitude(newValue);

			if (mOscSender != nullptr)
			{
				OSCEvent oscMessage("/flexblock/sinus/amplitude");
				oscMessage.addValue<float>(newValue);
				mOscSender->send(oscMessage);
			}
		});

		if (mOscSender != nullptr)
		{
			OSCEvent oscMessage("/flexblock/sinus/amplitude");
			oscMessage.addValue<float>(0.5f);
			mOscSender->send(oscMessage);
		}
	}


	void FlexblockGui::updateInput(int index, float value)
	{
		mFlexBlock->setMotorInput(index, value);
	}


	void FlexblockGui::updateOverride(int index, float value)
	{
		mFlexBlock->setOverrides(index, value);
	}


	void FlexblockGui::update(double deltaTime)
	{
		mTime += deltaTime;

		if (mHide)
			return;

		ImGui::BeginMainMenuBar();

		if (ImGui::BeginMenu("Windows"))
		{
			ImGui::MenuItem("Parameters", (const char*)0, &mProps.mShowParameters);
			ImGui::MenuItem("TimeLine", (const char*)0, &mProps.mShowTimeLine);
			ImGui::MenuItem("Motor Control", (const char*)0, &mProps.mShowMotorControl);
			ImGui::MenuItem("Information", (const char*)0, &mProps.mShowInformation);
			ImGui::MenuItem("Playlist", (const char*)0, &mProps.mShowPlaylist);
			ImGui::MenuItem("Show Motor Steps", (const char*)0, &mProps.mShowMotorSteps);
			ImGui::EndMenu();
		}

		bool resetLayout = false;
		ImGui::MenuItem("Reset Layout", (const char*)0, &resetLayout);
		ImGui::EndMainMenuBar();

		if (resetLayout)
		{
			ImGui::SetNextWindowPos(ImVec2(10, mWindowSize.y * 0.5f + 10));
			ImGui::SetNextWindowSize(ImVec2(mWindowSize.x * 0.5f - 20, mWindowSize.y * 0.4f - 20));
		}
		if (mProps.mShowParameters)
		{
			//
			ImGui::Begin("Parameters");
			for (const auto& group : mParameterService.getParameterGroups())
			{
				if (group.mGroup->mParameters.size() > 0)
				{
					if (ImGui::CollapsingHeader(group.mGroup->mID.c_str()))
					{
						for (const auto& parameterResource : group.mGroup->mParameters)
						{
							ParameterFloat* parameter = static_cast<ParameterFloat*>(parameterResource.get());

							std::string name = parameter->getDisplayName();

							float value = parameter->mValue;
							if (ImGui::SliderFloat(name.c_str(),
								&value,
								parameter->mMinimum,
								parameter->mMaximum))
							{
								parameter->setValue(value);
							}

							if (group.mGroup->mID == "Special Parameters")
							{
								// handle special parameters
								if (name == "Slack")
								{
									const float slackMinimum = mFlexBlock->getSlackMinimum();
									const float slackRange = mFlexBlock->getSlackRange();

									ImGui::SameLine();
									ImGui::Text("%.3f meter", value * slackRange + slackMinimum);
								}
								else if (name.find("Override") != std::string::npos)
								{
									const float overrideMinimum = mFlexBlock->getMotorOverrideMinimum();
									const float overrideRange = mFlexBlock->getMotorOverrideRange();

									ImGui::SameLine();
									ImGui::Text("%.3f meter", value * overrideRange + overrideMinimum);
								}
								else if (name == "Sinus Frequency")
								{
									const float sinusFrequency = mFlexBlock->getSinusFrequencyRange();

									ImGui::SameLine();
									ImGui::Text("%.1f hZ", value * sinusFrequency);
								}
								else if (name == "Sinus Amplitude")
								{
									const float sinusAmplitude = mFlexBlock->getSinusAmplitudeRange();

									ImGui::SameLine();
									ImGui::Text("%.3f meter", value * sinusAmplitude);
								}
							}
						}
					}
				}
			}
			ImGui::End();
		}

		if (resetLayout)
		{
			ImGui::SetNextWindowPos(ImVec2(10, mWindowSize.y * 0.9f + 10));
			ImGui::SetNextWindowSize(ImVec2(mWindowSize.x * 0.5f - 20, mWindowSize.y * 0.1f - 20));
		}
		if (mProps.mShowInformation)
			showInfoWindow();

		if (resetLayout)
		{
			ImGui::SetNextWindowPos(ImVec2(10, 30));
			ImGui::SetNextWindowContentSize(
				ImVec2(mProps.mChildWidth + 100.0f, mProps.mChildHeight + mProps.mChildHeight * (11.0f / 8.0f) + 200.0f));
		}
		if( mProps.mShowTimeLine)
			showTimeLineWindow();

		if (resetLayout)
		{
			ImGui::SetNextWindowPos(ImVec2(50, 50));
			ImGui::SetNextWindowSize(ImVec2(mWindowSize.x * 0.5f - 50, mWindowSize.y - 100));
		}
		if( mProps.mShowPlaylist )
			showPlaylist();

		if (resetLayout)
		{
			ImGui::SetNextWindowPos(ImVec2(100, 100));
			ImGui::SetNextWindowSize(ImVec2(mWindowSize.x * 0.5f, mWindowSize.y * 0.3f));
		}
		if( mProps.mShowMotorControl )
			showMotorControlWindow();

		if (mProps.mShowMotorSteps)
			showMotorSteps();
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
		static ImVec2 lastPosition = ImVec2(0, 0);

		// begin the window
		ImGui::Begin("Timeline", 0, ImGuiWindowFlags_HorizontalScrollbar );

		ImVec2 newPosition = ImGui::GetWindowPos();
		if (newPosition.x != lastPosition.x || newPosition.y != lastPosition.y)
		{
			lastPosition = newPosition;
			for (auto& pair : mProps.mDirtyFlags)
			{
				pair.second = true;
			}
		}

		bool needToOpenPopup = false;
		std::string popupIdToOpen = "";

		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 50));

		// draw timeline
		drawTimeline(needToOpenPopup, popupIdToOpen, "Motor Timeline", 8, 0);

		//
		drawTimeline(needToOpenPopup, popupIdToOpen, "Special Timeline", 11, 8);

		// draw player controls
		drawTimelinePlayerControls(needToOpenPopup, popupIdToOpen);

		//
		if (needToOpenPopup)
			ImGui::OpenPopup(popupIdToOpen.c_str());

		//
		handleLoadPopup();

		//
		handleSaveAsPopup();

		// 
		handleSequenceActionsPopup();

		// insertion popup
		handleInsertionPopup();

		//
		handleElementActionsPopup();

		//
		handleEditValuePopup();


		// release mouse
		if (ImGui::IsMouseReleased(0) && !mProps.mInPopup)
		{
			mProps.mSelectedElement = nullptr;
			mProps.mTangentPtr = nullptr;
			mProps.mCurvePtr = nullptr;
			mProps.mSelectedSequence = nullptr;
			mProps.mSelectedTimelineValue = 0;

			mProps.mCurrentAction = TimeLineActions::NONE;
		}

		ImGui::End();
	}


	void FlexblockGui::drawTimeline(
		bool& outPopupOpened, 
		std::string& outPopupId, 
		std::string timelineId,
		int size,
		int offset )
	{
		// we need to redraw the curves again after the first time drawing the timeline
		// this is because the window gets resized after drawing the timeline for the first time
		static bool firstTime = true;

		float windowWidth = ImGui::GetWindowWidth();
		float scrollX = ImGui::GetScrollX();

		// make this timeline dirty when it doesnt exist in the map yet
		if (mProps.mDirtyFlags.find(timelineId) == mProps.mDirtyFlags.end())
		{
			mProps.mDirtyFlags.insert({ timelineId, true });
		}

		// make the handler index
		if (mProps.mTimelineHandlerIndex.find(timelineId) == mProps.mTimelineHandlerIndex.end())
		{
			mProps.mTimelineHandlerIndex.insert({ timelineId, 0 });
		}

		// make the bool 
		if (mProps.mTimelineHeaderBools.find(timelineId) == mProps.mTimelineHeaderBools.end())
		{
			mProps.mTimelineHeaderBools.insert({ timelineId, false });
		}

		if (scrollX != mProps.mPrevScrollX)
		{
			for (auto& pair : mProps.mDirtyFlags)
			{
				pair.second = true;
			}
			mProps.mPrevScrollX = scrollX;
		}

		if (ImGui::GetScrollY() != mProps.mPrevScrollY)
		{
			mProps.mPrevScrollY = ImGui::GetScrollY();
			for (auto& pair : mProps.mDirtyFlags)
			{
				pair.second = true;
			}
		}

		// if header is clicked, mark this timeline dirty
		bool headerOpen = ImGui::CollapsingHeader(timelineId.c_str());
		if (mProps.mTimelineHeaderBools[timelineId])
		{
			if (headerOpen != mProps.mTimelineHeaderBools[timelineId])
			{
				mProps.mDirtyFlags[timelineId] = true;
				mProps.mTimelineHeaderBools[timelineId] = headerOpen;
			}
		}

		// draw the timeline
		if (headerOpen)
		{
			// begin timeline child
			ImGui::BeginChild(timelineId.c_str(), ImVec2(mProps.mChildWidth + 50, mProps.mChildHeight), false, ImGuiWindowFlags_NoMove);
			{
				float timeInDisplayStart = (math::max(0.0f, scrollX - 32) / mProps.mChildWidth) * mSequencePlayer->getDuration();
				float timeInDisplayEnd = timeInDisplayStart + (windowWidth / mProps.mChildWidth) * mSequencePlayer->getDuration();

				ImDrawList* drawList = ImGui::GetWindowDrawList();

				//
				const ImVec2 childSize = ImVec2(mProps.mChildWidth, mProps.mChildHeight - 125.0f);

				// start top left with a little bit of margin
				mProps.mTopLeftPosition = ImGui::GetCursorScreenPos();
				mProps.mTopLeftPosition.x += 50;
				mProps.mTopLeftPosition.y += 20;
				ImVec2 bottomRightPos = ImVec2(
					mProps.mTopLeftPosition.x + childSize.x,
					mProps.mTopLeftPosition.y + childSize.y);

				// timeline background
				drawList->AddRectFilled(mProps.mTopLeftPosition, bottomRightPos,
					colorBlack, 1.0f);

				// motors backgrounds
				for (int l = 0; l < size; l++)
				{
					drawList->AddRect(
						ImVec2(mProps.mTopLeftPosition.x, math::lerp<float>(mProps.mTopLeftPosition.y, bottomRightPos.y, (float)l / size)),
						ImVec2(bottomRightPos.x, math::lerp<float>(mProps.mTopLeftPosition.y, bottomRightPos.y, (float)(l + 1) / size)),
						colorWhite, 1.0f);
				}

				// right border
				drawList->AddLine(
					ImVec2(mProps.mTopLeftPosition.x + childSize.x, mProps.mTopLeftPosition.y),
					ImVec2(mProps.mTopLeftPosition.x + childSize.x, bottomRightPos.y),
					colorWhite);

				// draw timesteps on top of timeline
				const int timeStamps = 10;
				float timePart = (timeInDisplayEnd - timeInDisplayStart) / timeStamps;
				for (int i = 1; i < timeStamps; i++)
				{
					float time = (timeInDisplayStart + i * timePart);
					ImVec2 start(
						mProps.mTopLeftPosition.x + (time / mSequencePlayer->getDuration()) * mProps.mChildWidth,
						mProps.mTopLeftPosition.y);
					ImVec2 end = start;
					end.y -= 10;

					drawList->AddLine(start, end, colorWhite, 2.0f);

					ImVec2 textPos = end;
					textPos.x += 3;
					textPos.y -= 6;
					drawList->AddText(textPos, colorWhite, formatTimeString(time).c_str());
				}

				// needed to avoid elements texts from overlapping later on
				int yTextOffset = 0;

				// iterate through sequences 
				const auto & sequences = mSequencePlayer->getSequences();
				for (int i = 0; i < sequences.size(); i++)
				{
					float sequenceStartTime = sequences[i]->getStartTime();
					float sequenceEndTime = sequences[i]->getStartTime() + sequences[i]->getDuration();

					// cull sequence if not in display
					bool sequenceInDisplay =
						(sequenceStartTime > timeInDisplayStart && sequenceStartTime < timeInDisplayEnd) ||
						(sequenceEndTime > timeInDisplayStart && sequenceEndTime < timeInDisplayEnd) ||
						(sequenceStartTime < timeInDisplayStart && sequenceEndTime > timeInDisplayEnd) ||
						(timeInDisplayStart >= sequenceStartTime && timeInDisplayEnd <= sequenceEndTime);

					if (sequenceInDisplay)
					{
						float width = childSize.x * (sequences[i]->getDuration() / mSequencePlayer->getDuration());
						float startX = childSize.x * (sequences[i]->getStartTime() / mSequencePlayer->getDuration());

						// draw sequence line
						drawList->AddLine(
							ImVec2(mProps.mTopLeftPosition.x + startX, mProps.mTopLeftPosition.y),
							ImVec2(mProps.mTopLeftPosition.x + startX, mProps.mTopLeftPosition.y + childSize.y + 100),
							colorGreen);

						// draw sequence text
						drawList->AddText(
							ImVec2(mProps.mTopLeftPosition.x + startX + 5, mProps.mTopLeftPosition.y + childSize.y + 55),
							colorWhite,
							sequences[i]->mName.c_str());

						// draw sequence box
						ImVec2 sequenceBoxUpperLeft = ImVec2(mProps.mTopLeftPosition.x + startX, mProps.mTopLeftPosition.y + childSize.y + 75);
						ImVec2 sequenceBoxLowerRight = ImVec2(mProps.mTopLeftPosition.x + startX + 25, mProps.mTopLeftPosition.y + childSize.y + 100);

						drawList->AddRect(
							sequenceBoxUpperLeft,
							sequenceBoxLowerRight,
							colorGreen);

						// rename sequence action
						if (mProps.mCurrentAction == TimeLineActions::NONE)
						{
							if (ImGui::IsMouseHoveringRect(sequenceBoxUpperLeft, sequenceBoxLowerRight))
							{
								drawList->AddRectFilled(
									sequenceBoxUpperLeft,
									sequenceBoxLowerRight,
									colorGreen);

								if (ImGui::IsMouseClicked(1))
								{
									outPopupOpened = true;
									outPopupId = "SequenceActions";

									mProps.mSelectedSequence = sequences[i];
									mProps.mCurrentAction = TimeLineActions::SEQUENCE_ACTION_POPUP;
									mProps.mInPopup = true;
								}
								else
								{
									if (mProps.mShowToolTips)
									{
										ImGui::BeginTooltip();
										ImGui::Text("Right click to show sequence actions");
										ImGui::EndTooltip();
									}
								}
							}
						}

						// draw element lines and positions
						for (auto* element : sequences[i]->getElements())
						{
							float elementStartTime = element->getStartTime();
							float elementEndTime = element->getStartTime() + element->mDuration;

							// cull if element is not is display
							bool elementInDisplay =
								(elementStartTime > timeInDisplayStart && elementStartTime < timeInDisplayEnd) ||
								(elementEndTime > timeInDisplayStart && elementEndTime < timeInDisplayEnd) ||
								(elementStartTime < timeInDisplayStart && elementEndTime > timeInDisplayEnd) ||
								(timeInDisplayStart > elementStartTime && timeInDisplayEnd < elementEndTime);

							if (elementInDisplay)
							{
								bool drawLine = element != sequences[i]->getElements()[0];

								float elementPos = (element->getStartTime() - sequences[i]->getStartTime()) / sequences[i]->getDuration();
								float elementWidth = element->mDuration / sequences[i]->getDuration();

								// the bottom line
								if (drawLine)
								{
									drawList->AddLine(
										ImVec2(mProps.mTopLeftPosition.x + startX + width * elementPos, bottomRightPos.y),
										ImVec2(mProps.mTopLeftPosition.x + startX + width * elementPos, bottomRightPos.y + 50),
										colorLightGrey);

									// line in timeline
									drawList->AddLine(
										ImVec2(mProps.mTopLeftPosition.x + startX + width * elementPos, mProps.mTopLeftPosition.y),
										ImVec2(mProps.mTopLeftPosition.x + startX + width * elementPos, bottomRightPos.y + 50),
										colorLightGrey);
								}

								// the text
								drawList->AddText(
									ImVec2(mProps.mTopLeftPosition.x + startX + width * elementPos + 5, bottomRightPos.y + yTextOffset),
									colorLightGrey,
									element->mName.c_str());

								// draw dragger of element, changes duration of previous element
								{
									ImVec2 elementTimeDragRectStart(mProps.mTopLeftPosition.x + startX + width * elementPos, bottomRightPos.y + 40);
									ImVec2 elementTimeDragRectEnd(mProps.mTopLeftPosition.x + startX + width * elementPos + 10, bottomRightPos.y + 50);

									bool filled = false;
									if (mProps.mCurrentAction == TimeLineActions::NONE)
									{
										if (ImGui::IsMouseHoveringRect(
											elementTimeDragRectStart,
											elementTimeDragRectEnd, true))
										{
											filled = true;

											if (ImGui::IsMouseClicked(0))
											{
												mProps.mDirtyFlags[timelineId] = true;
												mProps.mCurrentAction = TimeLineActions::DRAGGING_ELEMENT;
												mProps.mSelectedElement = element;
											}
											else if (ImGui::IsMouseClicked(1))
											{
												// rename sequence action
												if (mProps.mCurrentAction == TimeLineActions::NONE)
												{
													outPopupOpened = true;
													outPopupId = "ElementActions";

													mProps.mSelectedElement = element;
													mProps.mCurrentAction = TimeLineActions::ELEMENT_ACTION_POPUP;
													mProps.mInPopup = true;
												}
											}
											else
											{
												if (mProps.mShowToolTips)
												{
													ImGui::BeginTooltip();
													ImGui::Text("Left click and hold to drag position, right click to show element actions");
													ImGui::EndTooltip();
												}
											}
										}
									}

									if (mProps.mCurrentAction == TimeLineActions::DRAGGING_ELEMENT
										&& ImGui::IsMouseDragging()
										&& element == mProps.mSelectedElement)
									{
										mProps.mDirtyFlags[timelineId] = true;

										filled = true;

										ImVec2 mousePs = ImGui::GetMousePos();
										double time = ((mousePs.x - mProps.mTopLeftPosition.x) / childSize.x) * mSequencePlayer->getDuration();

										auto* previousElement = element->getPreviousElement();
										if (previousElement != nullptr)
										{
											float newDuration = time - previousElement->getStartTime();

											previousElement->mDuration = newDuration;
											previousElement->mDuration = math::max(newDuration, 0.001f);
											mSequencePlayer->reconstruct();
										}
									}

									// draw rect in timeline
									if (!filled)
									{
										drawList->AddRect(
											elementTimeDragRectStart,
											elementTimeDragRectEnd,
											colorLightGrey);
									}
									else
									{
										drawList->AddRectFilled(
											elementTimeDragRectStart,
											elementTimeDragRectEnd,
											colorLightGrey);
									}
								}

								// set a height offset of the next text so they don't overlap to much
								yTextOffset += 20;
								if (yTextOffset > 50 - 10)
								{
									yTextOffset = 0;
								}

								bool drawHandlers = false;
								bool draggingMotor = false;
								// draw motor end positions in element
								{
									float motor_height = childSize.y / size;
									for (int m = 0; m < size; m++)
									{
										if ((mProps.mTimelineHandlerIndex[timelineId] >> m) & 1UL)
										{
											const float circleSize = 6.0f;

											bool filled = false;
											float x = mProps.mTopLeftPosition.x + startX + width * elementPos + width * elementWidth;
											float value = static_cast<ParameterFloat*>(element->getEndParameters()[offset + m])->mValue;
											float y = (bottomRightPos.y - motor_height * (float)m) - motor_height * value;

											// handle dragging of motor values
											if (mProps.mCurrentAction == TimeLineActions::NONE)
											{
												if (ImGui::IsMouseHoveringRect(ImVec2(x - 12, y - 12), ImVec2(x + 12, y + 12)))
												{
													filled = true;
													if (ImGui::IsMouseClicked(0))
													{
														mProps.mCurrentAction = TimeLineActions::DRAGGING_TIMELINEVALUE;
														mProps.mSelectedTimelineValue = m + offset;
														mProps.mSelectedElement = element;
														mProps.mDirtyFlags[timelineId] = true;
													}
													else if (ImGui::IsMouseClicked(1))
													{
														mProps.mCurrentAction = TimeLineActions::EDIT_VALUE_POPUP;
														mProps.mSelectedTimelineValue = m + offset;
														mProps.mSelectedElement = element;
														mProps.mDirtyFlags[timelineId] = true;
														mProps.mInPopup = true;
														outPopupOpened = true;
														outPopupId = "Edit Value";
													}
													else
													{
														ImGui::BeginTooltip();
														ImGui::Text("Hold left mouse button to drag \nPress right mouse button to edit value");
														ImGui::EndTooltip();
													}
												}
											}

											if (mProps.mCurrentAction == TimeLineActions::DRAGGING_TIMELINEVALUE
												&& ImGui::IsMouseDragging() &&
												m == mProps.mSelectedTimelineValue - offset &&
												element == mProps.mSelectedElement)
											{
												filled = true;
												mProps.mDirtyFlags[timelineId] = true;

												ImVec2 mousePos = ImGui::GetMousePos();

												float adjust = (mousePos.y - y) / motor_height;

												ParameterFloat* parameterFloat = static_cast<ParameterFloat*>(element->getEndParameters()[m+offset]);
												float newValue = parameterFloat->mValue;
												newValue -= adjust;
												newValue = math::clamp(newValue, parameterFloat->mMinimum, parameterFloat->mMaximum);
												parameterFloat->mValue = newValue;

												mSequencePlayer->reconstruct();
											}

											if (elementInDisplay)
											{
												if (!filled)
												{
													drawList->AddCircle(
														ImVec2(x, y),
														circleSize,
														colorRed,
														12,
														2.0f);
												}
												else
												{
													drawList->AddCircleFilled(
														ImVec2(x, y),
														circleSize,
														colorRed,
														12);
												}
											}
										}
									}
								}
								// width of this element in child
								float elementSizeWidth = elementWidth * width;

								// only applies to transition elements
								timeline::SequenceTransition* transition = dynamic_cast<timeline::SequenceTransition*>(element);
								if (transition != nullptr)
								{
									// get curves of this transition
									const auto& curves = transition->getCurves();

									// motor height is size of motor timeline in child height
									float motor_height = childSize.y / size;
									for (int m = 0; m < size; m++)
									{
										if ((mProps.mTimelineHandlerIndex[timelineId] >> m) & 1UL)
										{
											//
											mProps.mDirtyFlags[timelineId] = true;

											//
											ParameterFloat* startParameterFloat = static_cast<ParameterFloat*>(element->getStartParameters()[m+offset]);
											ParameterFloat* endParameterFloat = static_cast<ParameterFloat*>(element->getEndParameters()[m+offset]);

											//
											const float startValue = 0.0f;
											const float endValue = 1.0f;

											// get the range of the difference between start and finish
											const float range = endParameterFloat->mValue - startParameterFloat->mValue;

											// get the start position
											float start_curve = startParameterFloat->mValue;

											// is mouse hovering in this element part ?
											bool mouseInMotor = false;

											if (ImGui::IsMouseHoveringRect(
												ImVec2(mProps.mTopLeftPosition.x + startX + width * elementPos, bottomRightPos.y - motor_height * (m + 1)),
												ImVec2(mProps.mTopLeftPosition.x + startX + width * elementPos + elementSizeWidth, bottomRightPos.y - motor_height * m)))
											{
												mouseInMotor = true;

												if (mProps.mShowToolTips)
												{
													ImGui::BeginTooltip();
													ImGui::Text("Press C and left mouse button to add control points.\nPress X and left mouse to delete control points.\nLeft Click and drag control points and/or tangents.");
													ImGui::EndTooltip();
												}
											}

											// add control points
											if (mouseInMotor &&
												ImGui::IsMouseClicked(0) &&
												ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_C)) &&
												mProps.mCurrentAction == TimeLineActions::NONE)
											{
												mProps.mCurrentAction = TimeLineActions::ADD_CURVEPOINT;

												mProps.mDirtyFlags[timelineId] = true;

												// range can be negative, if curves moves downwards, thats why we need to flip the input later on
												bool flip = range < 0.0f;

												// to make sure p_y doesnt become infinite
												if (math::abs(range) > 0.00001f)
												{
													ImVec2 mousePos = ImGui::GetMousePos();

													// translate mouse pos to curve pos
													float pX = (mousePos.x - (mProps.mTopLeftPosition.x + startX + width * elementPos)) / elementSizeWidth;
													float pY = (((bottomRightPos.y - motor_height * (float)m) - mousePos.y) / motor_height) * range;

													if (flip)
													{
														pY = 1.0f + pY;
													}

													// create the new point
													auto newPoint = curves[m+offset]->mPoints[0];
													newPoint.mPos.mTime = pX;
													newPoint.mPos.mValue = math::clamp(pY, startValue, endValue);

													// add the point to the curve
													curves[m + offset]->mPoints.emplace_back(newPoint);

													// update curve
													curves[m + offset]->invalidate();
												}
											}

											// vector stores deletion of points
											std::vector<int> pointsToDelete;

											// iterate trough the points of the curve
											for (int p = 0; p < curves[m + offset]->mPoints.size(); p++)
											{
												// get a reference
												auto& point = curves[m + offset]->mPoints[p];

												// translate from element space to screen space
												float x = mProps.mTopLeftPosition.x + startX + width * elementPos + elementSizeWidth * point.mPos.mTime;
												float y = (bottomRightPos.y - motor_height * (float)m) - motor_height * ((point.mPos.mValue * range) + start_curve);

												// draw the position
												drawList->AddCircleFilled(
													ImVec2(x, y),
													3,
													colorWhite,
													12);

												// are we hovering inside the control point ?
												if (ImGui::IsMouseHoveringRect(ImVec2(x - 5, y - 5), ImVec2(x + 5, y + 5)) &&
													mProps.mCurrentAction == TimeLineActions::NONE)
												{
													if (ImGui::IsMouseClicked(0))
													{
														if (!ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_X)))
														{
															mProps.mCurvePtr = &point;
															mProps.mCurrentAction = TimeLineActions::DRAGGING_CURVEPOINT;
														}
														else
														{
															// handle deletion

															// store the index so we can delete it later
															pointsToDelete.emplace_back(p);

															mProps.mCurrentAction = TimeLineActions::DELETE_CURVEPOINT;
														}

														mProps.mDirtyFlags[timelineId] = true;
													}
												}

												//
												ImVec2 pointPos(x, y);

												// handle dragging of curve point
												if (mProps.mCurrentAction == TimeLineActions::DRAGGING_CURVEPOINT &&
													mProps.mCurvePtr == &point)
												{
													ImVec2 mousePos = ImGui::GetMousePos();

													// translate to curve space
													float adjust = (mousePos.y - y) / motor_height;

													point.mPos.mValue -= adjust * range;
													point.mPos.mValue = math::clamp(point.mPos.mValue, startValue, endValue);

													adjust = (mousePos.x - x) / elementSizeWidth;
													point.mPos.mTime += adjust;
													point.mPos.mTime = math::clamp(point.mPos.mTime, startValue, endValue);

													mProps.mDirtyFlags[timelineId] = true;
												}

												// translate 
												x = mProps.mTopLeftPosition.x + startX + width * elementPos + elementSizeWidth * (point.mPos.mTime + point.mOutTan.mTime);
												y = (bottomRightPos.y - motor_height * (float)m) - motor_height *  (((point.mPos.mValue + point.mOutTan.mValue) * range) + start_curve);

												ImVec2 inTangentPos(x, y);
												drawList->AddCircleFilled(
													inTangentPos,
													2,
													colorWhite,
													12);

												drawList->AddLine(pointPos, inTangentPos, colorLightGrey);

												// are we hovering this tangent ?
												if (ImGui::IsMouseHoveringRect(ImVec2(x - 5, y - 5), ImVec2(x + 5, y + 5)) &&
													mProps.mCurrentAction == TimeLineActions::NONE)
												{
													if (ImGui::IsMouseClicked(0))
													{
														mProps.mCurrentAction = TimeLineActions::DRAGGING_TANGENT;
														mProps.mTangentPtr = &point.mOutTan;

														mProps.mDirtyFlags[timelineId] = true;
													}
												}

												// handle dragging of tangent
												if (mProps.mCurrentAction == TimeLineActions::DRAGGING_TANGENT &&
													mProps.mTangentPtr == &point.mOutTan)
												{
													ImVec2 mousePos = ImGui::GetMousePos();

													// translate
													float adjust = (mousePos.y - y) / motor_height;
													mProps.mTangentPtr->mValue -= adjust * range;

													adjust = (mousePos.x - x) / elementSizeWidth;
													mProps.mTangentPtr->mTime += adjust;

													mProps.mDirtyFlags[timelineId] = true;
												}

												// translate
												x = mProps.mTopLeftPosition.x + startX + width * elementPos + elementSizeWidth * (point.mPos.mTime + point.mInTan.mTime);
												y = (bottomRightPos.y - motor_height * (float)m) - motor_height * (((point.mPos.mValue + point.mInTan.mValue) * range) + start_curve);
												ImVec2 outTangentPos(x, y);

												drawList->AddCircleFilled(
													outTangentPos,
													2,
													colorWhite,
													12);

												drawList->AddLine(pointPos, outTangentPos, colorLightGrey);

												// are we hovering this tangent ?
												if (ImGui::IsMouseHoveringRect(ImVec2(x - 5, y - 5), ImVec2(x + 5, y + 5)) &&
													mProps.mCurrentAction == TimeLineActions::NONE)
												{
													if (ImGui::IsMouseClicked(0))
													{
														mProps.mCurrentAction = TimeLineActions::DRAGGING_TANGENT;
														mProps.mTangentPtr = &point.mInTan;

														mProps.mDirtyFlags[timelineId] = true;
													}
												}

												// handle dragging
												if (mProps.mCurrentAction == TimeLineActions::DRAGGING_TANGENT &&
													mProps.mTangentPtr == &point.mInTan)
												{
													ImVec2 mousePos = ImGui::GetMousePos();

													float adjust = (mousePos.y - y) / motor_height;
													mProps.mTangentPtr->mValue -= adjust * range;

													adjust = (mousePos.x - x) / elementSizeWidth;
													mProps.mTangentPtr->mTime += adjust;

													mProps.mDirtyFlags[timelineId] = true;
												}

												// make sure tangents are linked
												if (mProps.mCurrentAction == TimeLineActions::DRAGGING_TANGENT &&
													mProps.mTangentPtr == &point.mOutTan)
												{
													point.mInTan.mTime = -mProps.mTangentPtr->mTime;
													point.mInTan.mValue = -mProps.mTangentPtr->mValue;

													curves[m + offset]->invalidate();
													mSequencePlayer->reconstruct();
												}

												if (mProps.mCurrentAction == TimeLineActions::DRAGGING_TANGENT &&
													mProps.mTangentPtr == &point.mInTan)
												{
													point.mOutTan.mTime = -mProps.mTangentPtr->mTime;
													point.mOutTan.mValue = -mProps.mTangentPtr->mValue;

													curves[m + offset]->invalidate();
													mSequencePlayer->reconstruct();
												}
											}

											// delete any points
											for (const int index : pointsToDelete)
											{
												curves[m + offset]->mPoints.erase(curves[m + offset]->mPoints.begin() + index);
												curves[m + offset]->invalidate();
											}
										}
									}
								}
							}
						}

						// draw value inputs 
						if (mProps.mDirtyFlags[timelineId])
						{
							//
							mProps.mCachedCurves[timelineId].clear();
							mProps.mCachedCurves[timelineId] = std::vector<std::vector<ImVec2>>(size);

							// create parameters that we evaluate
							std::vector<std::unique_ptr<ParameterFloat>> parametersPts;
							std::vector<Parameter*> parameters;
							for (int p = 0; p < size + offset; p++)
							{
								parametersPts.emplace_back(std::make_unique<ParameterFloat>());
								parameters.emplace_back(parametersPts.back().get());
							}

							// zoom in on the part that is shown in the window
							int steps = mProps.mCurveResolution;
							float part = windowWidth / mProps.mChildWidth;
							float partStart = math::clamp<float>(scrollX - 40, 0, mProps.mChildWidth) / mProps.mChildWidth;

							// start evaluating and create curves of motor
							for (int p = 0; p < steps; p++)
							{
								//
								mSequencePlayer->evaluate(((mSequencePlayer->getDuration() * part) / (float)steps) * (float)p + (mSequencePlayer->getDuration() * partStart), parameters);

								//
								for (int l = 0; l < size; l++)
								{
									float yPart = childSize.y / size;
									float yStart = yPart * l;

									mProps.mCachedCurves[timelineId][l].emplace_back(ImVec2(
										partStart * mProps.mChildWidth + mProps.mTopLeftPosition.x + childSize.x * part * (p * (1.0f / (float)steps)),
										bottomRightPos.y - yStart - yPart * static_cast<ParameterFloat*>(parameters[l+offset])->mValue));
								}
							}

							mProps.mDirtyFlags[timelineId] = false;
						}

						if (mProps.mCachedCurves[timelineId].size() == size)
						{
							for (int l = 0; l < size; l++)
							{
								if (mProps.mCachedCurves[timelineId][l].size() > 0)
								{
									// draw the polylines 
									drawList->AddPolyline(
										&*mProps.mCachedCurves[timelineId][l].begin(),
										mProps.mCachedCurves[timelineId][l].size(),
										colorRed,
										false,
										1.5f,
										false);
								}
							}
						}

					}
				}

				// draw edit curve toggle
				for (int i = 0; i < size; i++)
				{
					int motorId = (size-1) - i;
					float y_pos = (childSize.y / size) * i + 4;

					// draw motor text
					drawList->AddText(
						ImVec2(mProps.mTopLeftPosition.x - 50, mProps.mTopLeftPosition.y + y_pos),
						colorWhite,
						mProps.mParameterMap[static_cast<flexblock::PARAMETER_IDS>(motorId+offset)].c_str());

					//
					const ImVec2 rectTopLeft(mProps.mTopLeftPosition.x - 15, mProps.mTopLeftPosition.y + y_pos + 5);
					const ImVec2 rectBotRight(mProps.mTopLeftPosition.x - 5, mProps.mTopLeftPosition.y + y_pos + 15);

					bool filled = false;
					
					if (mProps.mCurrentAction == TimeLineActions::NONE)
					{
						if (ImGui::IsMouseHoveringRect(rectTopLeft, rectBotRight))
						{
							if (mProps.mShowToolTips)
							{
								ImGui::BeginTooltip();
								ImGui::Text("Left click to toggle curve edit mode");
								ImGui::EndTooltip();
							}

							filled = true;

							if (ImGui::IsMouseClicked(0))
							{
								if ((mProps.mTimelineHandlerIndex[timelineId] >> motorId) & 1UL)
								{
									mProps.mTimelineHandlerIndex[timelineId] &= ~(1UL << motorId);
									for (auto& pair : mProps.mTimelineHandlerIndex)
									{
										if (pair.first != timelineId)
										{
											pair.second = 0;
										}
									}
								}
								else
								{
									mProps.mTimelineHandlerIndex[timelineId] |= 1UL << motorId;
									for (auto& pair : mProps.mTimelineHandlerIndex)
									{
										if (pair.first != timelineId)
										{
											pair.second = 0;
										}
									}
								}
								mProps.mCurrentAction = TimeLineActions::ENABLING_HANDLERS;
							}
						}
					}

					if (!filled)
						filled = (mProps.mTimelineHandlerIndex[timelineId] >> motorId) & 1UL;

					if (!filled)
					{
						drawList->AddRect(
							rectTopLeft,
							rectBotRight,
							colorWhite);
					}
					else
					{
						drawList->AddRectFilled(
							rectTopLeft,
							rectBotRight,
							colorWhite);
					}
				}

				// draw player position 
				float player_pos = (mSequencePlayer->getCurrentTime() / mSequencePlayer->getDuration()) * childSize.x;
				drawList->AddLine(
					ImVec2(mProps.mTopLeftPosition.x + player_pos, mProps.mTopLeftPosition.y),
					ImVec2(mProps.mTopLeftPosition.x + player_pos, bottomRightPos.y),
					colorWhite);

				// time in seconds
				drawList->AddText(
					ImVec2(mProps.mTopLeftPosition.x + player_pos + 5, mProps.mTopLeftPosition.y - 20),
					colorRed,
					formatTimeString(mSequencePlayer->getCurrentTime()).c_str());

				// handle dragging of timeline
				if (ImGui::IsMouseHoveringRect(mProps.mTopLeftPosition, bottomRightPos))
				{
					mProps.mDrawMouseCursorInTimeline = true;
					if (!mProps.mInPopup)
					{
						ImVec2 mousePos = ImGui::GetMousePos();
						mousePos = ImVec2(mousePos.x - mProps.mTopLeftPosition.x, mousePos.y - mProps.mTopLeftPosition.y);
						float pos = mousePos.x / childSize.x;
						mProps.mCurrentTimeOfMouseInSequence = pos * mSequencePlayer->getDuration();

						// set cursorpos 
						mProps.mMouseCursorPositionInTimeline = (mProps.mCurrentTimeOfMouseInSequence / mSequencePlayer->getDuration()) * childSize.x;

						if (mProps.mCurrentAction == TimeLineActions::NONE)
						{
							// is clicked inside timeline ? jump to position
							if (ImGui::IsMouseClicked(0))
							{
								mProps.mCurrentAction = TimeLineActions::DRAGGING_PLAYERPOSITION;

								mSequencePlayer->setTime(mProps.mCurrentTimeOfMouseInSequence);
							}
						}

						if (mProps.mCurrentAction == TimeLineActions::DRAGGING_PLAYERPOSITION)
						{
							// handle drag in timeline
							if (ImGui::IsMouseDragging())
							{
								// translate mouseposition to position in sequenceplayer
								ImVec2 mousePos = ImGui::GetMousePos();
								mousePos = ImVec2(mousePos.x - mProps.mTopLeftPosition.x, mousePos.y - mProps.mTopLeftPosition.y);
								float pos = mousePos.x / childSize.x;
								mProps.mCurrentTimeOfMouseInSequence = pos * mSequencePlayer->getDuration();
								mSequencePlayer->setTime(mProps.mCurrentTimeOfMouseInSequence);
							}
						}
						else
						{
							if (mProps.mShowToolTips && !mProps.mInPopup)
							{
								ImGui::BeginTooltip();
								ImGui::Text("Left click to jump to position, hold to drag/scrub position.\nRight click to add elements/sequences.");
								ImGui::EndTooltip();
							}
						}
					}
				}
				else
				{
					if (!mProps.mInPopup)
					{
						mProps.mDrawMouseCursorInTimeline = false;
					}
				}

				// draw position of mouse ( cursor ) in timeline
				if (mProps.mDrawMouseCursorInTimeline)
				{
					drawList->AddLine(
						ImVec2(mProps.mTopLeftPosition.x + mProps.mMouseCursorPositionInTimeline, mProps.mTopLeftPosition.y),
						ImVec2(mProps.mTopLeftPosition.x + mProps.mMouseCursorPositionInTimeline, bottomRightPos.y),
						colorRed, 1.5f);

					// time in seconds
					drawList->AddText(
						ImVec2(mProps.mTopLeftPosition.x + mProps.mMouseCursorPositionInTimeline + 5, mProps.mTopLeftPosition.y - 20),
						colorRed,
						formatTimeString(mProps.mCurrentTimeOfMouseInSequence).c_str());
				}

				// handle insertion of elements or sequences
				if (mProps.mCurrentAction == TimeLineActions::NONE &&
					ImGui::IsMouseClicked(1) &&
					ImGui::IsMouseHoveringRect(mProps.mTopLeftPosition, bottomRightPos))
				{
					mProps.mInPopup = true;
					mProps.mCurrentAction = TimeLineActions::INSERTION_POPUP;

					outPopupOpened = true;
					outPopupId = "Insert";
				}
				ImGui::EndChild();
			}

			// make sure the curves get cached again after they are drawn the first time
			if (firstTime)
			{
				firstTime = false;
				mProps.mDirtyFlags[timelineId] = true;
			}
		}
	}


	void FlexblockGui::drawTimelinePlayerControls(bool& outPopupOpened, std::string& popupId)
	{
		static auto showTip = [this](const char* tip)
		{
			if (mProps.mShowToolTips)
			{
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text(tip);
					ImGui::EndTooltip();
				}
			}
		};

		// make sure the top elements above the timeline scroll together with the scrollbar so only timeline moves
		ImGui::SetCursorPos(ImVec2(ImGui::GetScrollX(), 27 + ImGui::GetScrollY()));

		//
		ImGui::BeginChild("Controls", ImVec2( 2000, 50 ), false, ImGuiWindowFlags_NoScrollbar);

		ImGui::Spacing();
		ImGui::Indent();

		// stop button
		if (ImGui::Button("Stop"))
		{
			mSequencePlayer->stop();
		}

		showTip("Stops the player");

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

				showTip("Pause the player");
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

				showTip("Starts the player");
			}
		}

		ImGui::SameLine();

		// loop
		bool isLooping = mSequencePlayer->getIsLooping();
		if (ImGui::Checkbox("Loop", &isLooping))
		{
			mSequencePlayer->setIsLooping(isLooping);
		}

		showTip("Toggle loop");

		// follow player position
		ImGui::SameLine();
		ImGui::Checkbox("Follow", &mProps.mFollowPlayer);
		if (mProps.mFollowPlayer)
		{
			for (auto& pair : mProps.mDirtyFlags)
			{
				pair.second = true;
			}

			ImGui::SetScrollX((mSequencePlayer->getCurrentTime() / mSequencePlayer->getDuration()) * mProps.mChildWidth);
		}

		showTip("Follow the player position with the scrollbar, keeps the player position in window");

		// speed
		ImGui::SameLine();
		float speed = mSequencePlayer->getSpeed();
		ImGui::PushItemWidth(50.0f);
		ImGui::DragFloat("Speed", &speed, 0.05f, -5.0f, 5.0f);
		ImGui::PopItemWidth();
		mSequencePlayer->setSpeed(speed);

		showTip("set playback speed");

		// zoom of timeline
		ImGui::SameLine();
		ImGui::PushItemWidth(100.0f);
		if (ImGui::SliderFloat("Vertical Zoom", &mProps.mChildHeight, 350.0f, 1500.0f, ""))
		{
			for (auto& pair : mProps.mDirtyFlags)
			{
				pair.second = true;
			}
		}
			

		ImGui::PopItemWidth();

		showTip("Vertical zoom");

		// zoom of timeline
		ImGui::SameLine();
		ImGui::PushItemWidth(100.0f);
		if (ImGui::SliderFloat("Horizontal Zoom", &mProps.mLengthPerSecond, 4.0f, 900.0f, ""))
		{
			for (auto& pair : mProps.mDirtyFlags)
			{
				pair.second = true;
			}
		}
			
		ImGui::PopItemWidth();

		mProps.mChildWidth = (mSequencePlayer->getDuration() / mProps.mLengthPerSecond) * ImGui::GetWindowWidth();
		
		showTip("Horizontal zoom");

		// curves resolution
		ImGui::SameLine();
		ImGui::PushItemWidth(100.0f);
		if (ImGui::SliderInt("Curve Res.", &mProps.mCurveResolution, 50, 666, ""))
		{
			for (auto& pair : mProps.mDirtyFlags)
			{
				pair.second = true;
			}
		}
			
		ImGui::PopItemWidth();

		showTip("Resolution of curve, higher means a more smooth curve but heavier on rendering/generating");

		// handle save button
		ImGui::SameLine();

		if (ImGui::Button("Save"))
		{
			std::string currentShowName = mSequencePlayer->getShowName();
			currentShowName += ".json";

			utility::ErrorState errorState;
			if (mSequencePlayer->save(currentShowName, errorState))
			{
				mSequencePlayer->load("shows/" + currentShowName, errorState);
			}
		}

		showTip("Save current show");

		ImGui::SameLine();

		if (ImGui::Button("Save As"))
		{
			popupId = "Save As";

			mProps.mInPopup = true;
			outPopupOpened = true;
			mProps.mCurrentAction = TimeLineActions::SAVE_POPUP;
		}

		showTip("Save as new show");

		// handle load
		ImGui::SameLine();
		if (ImGui::Button("Load"))
		{
			popupId = "Load";
			mProps.mInPopup = true;
			outPopupOpened = true;
			mProps.mCurrentAction = TimeLineActions::LOAD_POPUP;
			for (auto& pair : mProps.mDirtyFlags)
			{
				pair.second = true;
			}
		}

		showTip("Load show");

		//
		ImGui::SameLine();
		ImGui::Text(("CurrentShow : " + mSequencePlayer->getShowName()).c_str());

		//
		ImGui::SameLine();
		ImGui::Checkbox("Show tooltips", &mProps.mShowToolTips);

		showTip("Toggle tooltips");

		ImGui::SameLine();
		if (ImGui::Checkbox("Show PlayList", &mProps.mShowPlaylist))
		{
			if (mProps.mShowPlaylist)
			{
				ImGui::SetNextTreeNodeOpen(true);
			}
		}

		showTip("Show list of sequences");

		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - ImGui::GetScrollY()));
	}


	void FlexblockGui::handleLoadPopup()
	{
		//
		if (ImGui::BeginPopupModal("Load", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			const std::string showDir = "Shows";

			// Find all files in the preset directory
			std::vector<std::string> files_in_directory;
			utility::listDir(showDir.c_str(), files_in_directory);

			std::vector<std::string> shows;
			for (const auto& filename : files_in_directory)
			{
				// Ignore directories
				if (utility::dirExists(filename))
					continue;

				if (utility::getFileExtension(filename) == "json")
				{
					shows.push_back(utility::getFileName(filename));
				}
			}

			ImGui::Combo("Shows", &mProps.mSelectedShowIndex, [](void* data, int index, const char** out_text)
			{
				ParameterService::PresetFileList* show_files = (ParameterService::PresetFileList*)data;
				*out_text = (*show_files)[index].data();
				return true;
			}, &shows, shows.size());

			utility::ErrorState errorState;
			if (ImGui::Button("Load"))
			{
				if (mSequencePlayer->load(files_in_directory[mProps.mSelectedShowIndex], errorState))
				{
					for (auto& pair : mProps.mDirtyFlags)
					{
						pair.second = true;
					}

					ImGui::CloseCurrentPopup();
					mProps.mInPopup = false;
					mProps.mCurrentAction = TimeLineActions::NONE;
				}
				else
				{
					mProps.mErrorString = errorState.toString();
					ImGui::OpenPopup("Failed to load show");
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
				mProps.mInPopup = false;
				mProps.mCurrentAction = TimeLineActions::NONE;
			}

			if (ImGui::BeginPopupModal("Failed to load show", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text(mProps.mErrorString.c_str());
				if (ImGui::Button("OK"))
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			ImGui::EndPopup();
		}
	}


	void FlexblockGui::handleSequenceActionsPopup()
	{
		if (ImGui::BeginPopup("SequenceActions"))
		{
			char buffer[256];

			strcpy(*&buffer, mProps.mSelectedSequence->mName.c_str());

			if (ImGui::InputText("Rename", *&buffer, 256))
			{
				std::string newName(buffer);
				mProps.mSelectedSequence->mName = newName;
			}

			if (mSequencePlayer->getSequences().size() > 1)
			{
				if (ImGui::Button("Delete"))
				{
					mSequencePlayer->removeSequence(mProps.mSelectedSequence);

					for (auto& pair : mProps.mDirtyFlags)
					{
						pair.second = true;
					}

					mProps.mInPopup = false;
					mProps.mCurrentAction = TimeLineActions::NONE;
					ImGui::CloseCurrentPopup();
				}
			}

			if (ImGui::Button("Done"))
			{
				mProps.mInPopup = false;
				mProps.mCurrentAction = TimeLineActions::NONE;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		else
		{
			if (mProps.mCurrentAction == TimeLineActions::SEQUENCE_ACTION_POPUP)
			{
				mProps.mCurrentAction = TimeLineActions::NONE;
				mProps.mInPopup = false;
			}
		}
	}


	void FlexblockGui::handleSaveAsPopup()
	{
		// save as popup
		if (ImGui::BeginPopupModal("Save As", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			const std::string showDir = "Shows";

			// Find all files in the preset directory
			std::vector<std::string> files_in_directory;
			utility::listDir(showDir.c_str(), files_in_directory);

			std::vector<std::string> shows;
			for (const auto& filename : files_in_directory)
			{
				// Ignore directories
				if (utility::dirExists(filename))
					continue;

				if (utility::getFileExtension(filename) == "json")
				{
					shows.push_back(utility::getFileName(filename));
				}
			}
			shows.push_back("<New...>");

			if (ImGui::Combo("Shows", &mProps.mSelectedShowIndex, [](void* data, int index, const char** out_text)
			{
				ParameterService::PresetFileList* show_files = (ParameterService::PresetFileList*)data;
				*out_text = (*show_files)[index].data();

				return true;
			}, &shows, shows.size()))
			{
				if (mProps.mSelectedShowIndex == shows.size() - 1)
				{
					ImGui::OpenPopup("New");
				}
				else
				{
					ImGui::OpenPopup("Overwrite");
				}
			}

			std::string newFilename;
			utility::ErrorState errorState;
			if (handleNewShowPopup(newFilename, errorState))
			{
				// Insert before the '<new...>' item
				shows.insert(shows.end() - 1, newFilename);
				if (mSequencePlayer->save(newFilename, errorState))
				{
					mProps.mSelectedShowIndex = shows.size() - 1;

					if (mSequencePlayer->load(newFilename, errorState))
					{

					}
					else
					{
						mProps.mErrorString = errorState.toString();
						ImGui::OpenPopup("Failed to load show");
					}
				}
				else
				{
					mProps.mErrorString = errorState.toString();
					ImGui::OpenPopup("Failed to save show");
				}
			}

			if (ImGui::BeginPopupModal("Overwrite"))
			{
				ImGui::Text(("Are you sure you want to overwrite " + shows[mProps.mSelectedShowIndex] + " ?").c_str());
				if (ImGui::Button("OK"))
				{
					if (mSequencePlayer->save(shows[mProps.mSelectedShowIndex], errorState))
					{
						if (mSequencePlayer->load(shows[mProps.mSelectedShowIndex], errorState))
						{

						}
						else
						{
							mProps.mErrorString = errorState.toString();
							ImGui::OpenPopup("Failed  to save show");
						}
					}
					else
					{
						mProps.mErrorString = errorState.toString();
						ImGui::OpenPopup("Failed to save show");
					}

					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			if (ImGui::BeginPopupModal("Failed to save show"))
			{
				ImGui::Text(mProps.mErrorString.c_str());
				if (ImGui::Button("OK"))
				{
					ImGui::CloseCurrentPopup();
				}
				mProps.mInPopup = false;
				mProps.mCurrentAction = TimeLineActions::NONE;
				ImGui::EndPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Done"))
			{
				mProps.mInPopup = false;
				mProps.mCurrentAction = TimeLineActions::NONE;
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			ImGui::EndPopup();
		}
	}


	template<typename T1>
	std::string FlexblockGui::convertToString(T1 number, int precision)
	{
		std::ostringstream streamObj;
		streamObj << std::fixed;
		streamObj << std::setprecision(precision) << number;
		return streamObj.str();
	}


	std::string FlexblockGui::formatTimeString(float time)
	{
		int hours = time / 3600.0f;
		int minutes = (int) (time / 60.0f) % 60;
		int seconds = (int) time % 60;
		int milliseconds = (int)(time * 100.0f) % 100;

		std::stringstream stringStream;

		stringStream << std::setw(2) << std::setfill('0') << seconds;
		std::string secondsString = stringStream.str();

		stringStream = std::stringstream();
		stringStream << std::setw(2) << std::setfill('0') << minutes;
		std::string minutesString = stringStream.str();

		stringStream = std::stringstream();
		stringStream << std::setw(2) << std::setfill('0') << milliseconds;
		std::string millisecondsStrings = stringStream.str();

		std::string hoursString = "";
		if (hours > 0)
		{
			stringStream = std::stringstream();
			stringStream << std::setw(2) << std::setfill('0') << hours;
			hoursString = stringStream.str() + ":";
		}

		return hoursString + minutesString + ":" + secondsString + ":" + millisecondsStrings;
	}


	void FlexblockGui::showPlaylist()
	{
		// Fetch resource manager to get access to all loaded resources
		ResourceManager* resourceManager = mApp.getCore().getResourceManager();

		ImGui::Begin("Playlist");
		ImGui::Spacing();

		ImGui::Spacing();

		if (ImGui::TreeNode("Playlist"))
		{
			const auto& sequences = mSequencePlayer->getSequences();

			for (const auto* sequence : sequences)
			{
				ImGui::PushID((void*)sequence);

				ImVec4 color = ImGui::ColorConvertU32ToFloat4( colorWhite );

				bool isSequenceBeingPlayed = mSequencePlayer->getCurrentSequence() == sequence;
				if (isSequenceBeingPlayed)
				{
					color = ImGui::ColorConvertU32ToFloat4( colorRed );
				}

				ImGui::PushStyleColor(0, color);
				if(ImGui::Button(sequence->mName.c_str()))
				{
					mSequencePlayer->skipToSequence(sequence);
				}
				ImGui::SameLine();
				ImGui::Text(formatTimeString(sequence->getStartTime()).c_str());
				ImGui::PopStyleColor();

				const auto& elements = sequence->getElements();

				ImGui::Indent();
				for (const auto* element : elements)
				{
					ImGui::PushID((void*)element);

					bool isElementBeingPlayed =
						isSequenceBeingPlayed && (
							mSequencePlayer->getCurrentTime() >= element->getStartTime() &&
							mSequencePlayer->getCurrentTime() < element->getStartTime() + element->mDuration);

					if (isElementBeingPlayed)
						ImGui::PushStyleColor(0, color);

					if (ImGui::SmallButton(element->mName.c_str()))
					{
						//mSequencePlayer->skipToSequence(sequence);
						mSequencePlayer->setTime(element->getStartTime());
					}
					ImGui::SameLine();
					ImGui::Text(formatTimeString(element->getStartTime()).c_str());

					if (isElementBeingPlayed)
					{
						float progress = (mSequencePlayer->getCurrentTime() - element->getStartTime()) / element->mDuration;
						int percentage = int(progress * 100.0f);

						ImGui::SameLine();
						ImGui::Text("   %i%s", percentage, "%");
					}

					if (isElementBeingPlayed)
						ImGui::PopStyleColor();

					ImGui::PopID();
				}
				ImGui::Unindent();
				ImGui::PopID();

			}
			ImGui::TreePop();
		}

		ImGui::End();
	}


	void FlexblockGui::handleElementActionsPopup()
	{
		if (ImGui::BeginPopup("ElementActions"))
		{
			char buffer[256];

			strcpy(*&buffer, mProps.mSelectedElement->mName.c_str());

			if (ImGui::InputText("Rename", *&buffer, 256))
			{
				std::string newName(buffer);
				mProps.mSelectedElement->mName = newName;
			}

			const timeline::Sequence* owningSequence 
				= mSequencePlayer->getSequenceAtTime(mProps.mSelectedElement->getStartTime());

			if (ImGui::InputFloat("Duration", &mProps.mSelectedElement->mDuration, 0.1f, 0.2f, 2) )
			{
				if (mProps.mSelectedElement->mDuration < 0.01f)
				{
					mProps.mSelectedElement->mDuration = 0.01f;
				}

				mSequencePlayer->reconstruct();
				for (auto& pair : mProps.mDirtyFlags)
				{
					pair.second = true;
				}
			}

			// enable delete if sequence has more then one element
			if (owningSequence != nullptr &&
				owningSequence->getElements().size() > 1)
			{
				if (ImGui::Button("Delete"))
				{
					mSequencePlayer->removeSequenceElement(owningSequence, mProps.mSelectedElement);
					for (auto& pair : mProps.mDirtyFlags)
					{
						pair.second = true;
					}

					mProps.mInPopup = false;
					mProps.mCurrentAction = TimeLineActions::NONE;
					ImGui::CloseCurrentPopup();
				}
			}

			if (ImGui::Button("Done"))
			{
				mProps.mInPopup = false;
				mProps.mCurrentAction = TimeLineActions::NONE;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		else
		{
			if (mProps.mCurrentAction == TimeLineActions::ELEMENT_ACTION_POPUP)
			{
				mProps.mCurrentAction = TimeLineActions::NONE;
				mProps.mInPopup = false;
			}
		}
	}


	void FlexblockGui::showInfoWindow()
	{
		// Color used for highlights
		mApp.getCore().getFramerate();

		ImGui::Begin("Information");
		getCurrentDateTime(mDateTime);
		ImGui::Text(mDateTime.toString().c_str());
		ImGui::SameLine();
		ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(colorRed), "%.3f ms/frame (%.1f FPS)", 1000.0f / mApp.getCore().getFramerate(), mApp.getCore().getFramerate());
		ImGui::End();
	}


	void FlexblockGui::handleInsertionPopup()
	{
		if (ImGui::BeginPopup("Insert"))
		{
			bool openElementActionPopup = false;
			bool openSequenceActionPopup = false;

			// insert pause element at current time
			if (ImGui::Button("Insert Pause"))
			{
				utility::ErrorState errorState;
				std::unique_ptr<timeline::SequencePause> elementUniquePtr = std::make_unique<timeline::SequencePause>();
				timeline::SequencePause* elementPtr = elementUniquePtr.get();

				bool success = insertNewElement(std::move(elementUniquePtr), errorState);

				if (!success)
				{
					// handle error
					ImGui::OpenPopup("Insert Error");
					mProps.mErrorString = errorState.toString();
				}
				else 
				{
					// exit popup
					mProps.mInPopup = false;
					mProps.mCurrentAction = TimeLineActions::NONE;
					ImGui::CloseCurrentPopup();

					//
					openElementActionPopup = true;
					mProps.mSelectedElement = elementPtr;
				}
			}

			// insert transition element
			if (ImGui::Button("Insert Transition"))
			{
				utility::ErrorState errorState;

				flexblock::FlexblockSequenceTransition* elementPtr = nullptr;
				std::unique_ptr<flexblock::FlexblockSequenceTransition> elementUniquePtr 
					= std::make_unique<flexblock::FlexblockSequenceTransition>();
				elementPtr = elementUniquePtr.get();

				bool success = insertNewElement(std::move(elementUniquePtr), errorState);

				if (!success)
				{
					// handle error
					ImGui::OpenPopup("Insert Error");
					mProps.mErrorString = errorState.toString();
				}
				else
				{
					// exit popup
					mProps.mInPopup = false;
					mProps.mCurrentAction = TimeLineActions::NONE;
					ImGui::CloseCurrentPopup();

					mProps.mSelectedElement = elementPtr;
					openElementActionPopup = true;
				}
			}

			// insert new sequence
			if (ImGui::Button("Insert Sequence"))
			{
				utility::ErrorState errorState;

				std::unique_ptr<flexblock::FlexblockSequence> sequenceUniquePtr
					= std::make_unique<flexblock::FlexblockSequence>();
				flexblock::FlexblockSequence* sequencePtr = sequenceUniquePtr.get();

				if (!insertNewSequence(std::move(sequenceUniquePtr), errorState))
				{
					ImGui::OpenPopup("Insert Error");
					mProps.mErrorString = errorState.toString();
				}
				else
				{
					mProps.mInPopup = false;
					mProps.mCurrentAction = TimeLineActions::NONE;
					ImGui::CloseCurrentPopup();

					mProps.mSelectedSequence = sequencePtr;
					openSequenceActionPopup = true;
				}
			}

			if (ImGui::Button("Cancel"))
			{
				mProps.mInPopup = false;
				mProps.mCurrentAction = TimeLineActions::NONE;
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::BeginPopupModal("Insert Error"))
			{
				ImGui::Text(mProps.mErrorString.c_str());

				if (ImGui::Button("Ok"))
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			ImGui::EndPopup();

			if (openElementActionPopup)
			{
				mProps.mInPopup = true;
				mProps.mCurrentAction = TimeLineActions::ELEMENT_ACTION_POPUP;
				ImGui::OpenPopup("ElementActions");
			}

			if (openSequenceActionPopup)
			{
				mProps.mInPopup = true;
				mProps.mCurrentAction = TimeLineActions::SEQUENCE_ACTION_POPUP;
				ImGui::OpenPopup("SequenceActions");
			}
		}
		else
		{
			if (mProps.mCurrentAction == TimeLineActions::INSERTION_POPUP)
			{
				mProps.mCurrentAction = TimeLineActions::NONE;
				mProps.mInPopup = false;
			}
		}
	}


	void FlexblockGui::handleEditValuePopup()
	{
		if (ImGui::BeginPopup("Edit Value"))
		{
			ImGui::Text("Edit value");

			if (mProps.mSelectedElement != nullptr)
			{
				ParameterFloat* parameterFloat =
					dynamic_cast<ParameterFloat*>(mProps.mSelectedElement->getEndParameters()[mProps.mSelectedTimelineValue]);

				if (parameterFloat != nullptr)
				{
					float value = parameterFloat->mValue;
					ImGui::InputFloat("", &value, 0.001f, 0.01f, 3);
					value = math::clamp(value, 0.0f, 1.0f);
					parameterFloat->setValue(value);

					auto parameterID = static_cast<flexblock::PARAMETER_IDS>(mProps.mSelectedTimelineValue);
					switch (parameterID)
					{
					case nap::flexblock::SLACK:
					{
						const float slackRange = mFlexBlock->getSlackRange();
						const float slackMinimum = mFlexBlock->getSlackMinimum();

						ImGui::SameLine();
						ImGui::Text("%.3f meter", value * slackRange + slackMinimum);
					}
						break;
					case nap::flexblock::MOTOR_OVERRIDE_ONE:
					case nap::flexblock::MOTOR_OVERRIDE_TWO:
					case nap::flexblock::MOTOR_OVERRIDE_THREE:
					case nap::flexblock::MOTOR_OVERRIDE_FOUR:
					case nap::flexblock::MOTOR_OVERRIDE_FIVE:
					case nap::flexblock::MOTOR_OVERRIDE_SIX:
					case nap::flexblock::MOTOR_OVERRIDE_SEVEN:
					case nap::flexblock::MOTOR_OVERRIDE_EIGHT:
					{
						const float motorOverrideMinimum = mFlexBlock->getMotorOverrideMinimum();
						const float motorOverrideRange = mFlexBlock->getMotorOverrideRange();

						ImGui::SameLine();
						ImGui::Text("%.3f meter", value * motorOverrideRange + motorOverrideMinimum);
					}
						break;
					case nap::flexblock::SINUS_FREQUENCY:
					{
						ImGui::SameLine();
						ImGui::Text("%.1f Hz", value * mFlexBlock->getSinusFrequencyRange());
					}
						break;
					case nap::flexblock::SINUS_AMPLITUDE:
					{
						ImGui::SameLine();
						ImGui::Text("%.3f meter", value * mFlexBlock->getSinusAmplitudeRange());
					}
						break;
					default:
						break;
					}
				}
			}

			if (ImGui::Button("Done"))
			{
				mProps.mInPopup = false;
				mProps.mCurrentAction = TimeLineActions::NONE;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		else
		{
			if (mProps.mCurrentAction == TimeLineActions::EDIT_VALUE_POPUP)
			{
				mProps.mCurrentAction = TimeLineActions::NONE;
				mProps.mInPopup = false;
			}
		}
	}


	bool FlexblockGui::handleNewShowPopup(std::string& outNewFilename, utility::ErrorState& error)
	{
		bool result = false;

		if (ImGui::BeginPopupModal("New", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			static char name[256] = { 0 };
			ImGui::InputText("Name", name, 256);

			if (ImGui::Button("OK") && strlen(name) != 0)
			{
				outNewFilename = std::string(name, strlen(name));
				outNewFilename += ".json";
				ImGui::CloseCurrentPopup();
				result = true;
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}

		return result;
	}


	bool FlexblockGui::insertNewElement(std::unique_ptr<timeline::SequenceElement> newElement, utility::ErrorState& errorState)
	{
		// retrieve current sequence and element
		double time = mProps.mCurrentTimeOfMouseInSequence;
		timeline::Sequence* sequence = mSequencePlayer->getSequenceAtTime(time);
		timeline::SequenceElement* element = sequence->getElementAtTime(time);

		// make sure curves get re-evaluated
		for (auto& pair : mProps.mDirtyFlags)
		{
			pair.second = true;
		}

		// check if element is not to close to other element already existing..
		for (const auto* sequence : mSequencePlayer->getSequences())
		{
			for (const auto* element : sequence->getElements())
			{
				float diff = math::abs((float)time - (float)element->getStartTime());
				if (!errorState.check(
					diff > 0.05f,
					"New element to close to element [%s] in sequence [%s]", 
					sequence->mName.c_str(),
					element->mName.c_str() )) // to close
				{
					return false;
				}
			}
		}

		// calculate new duration of current element
		// delta duration is used to set the time of the new element
		float newDuration = time - element->getStartTime();
		float deltaDuration = element->mDuration - newDuration;
		element->mDuration = newDuration;

		// generate the new element
		newElement->mID = "GeneratedElement" + getTimeString();

		// set duration and start of new element
		newElement->mDuration = deltaDuration;
		newElement->setStartTime(time);

		// init new element
		if (!newElement->init(errorState))
		{
			// handle error
			return false;
		}
		else
		{
			// insert element in sequence and transfer ownership
			auto* element = sequence->insertElement(std::move(newElement));

			//
			if (element->getPreviousElement() != nullptr)
			{
				auto* previousElement = element->getPreviousElement();

				element->setStartParameters(previousElement->getEndParameters());
			}
			else
			{
				element->setStartParameters(sequence->getStartParameters());
			}

			// reconstruct the sequence
			mSequencePlayer->reconstruct();


		}

		return true;
	}


	bool FlexblockGui::insertNewSequence(std::unique_ptr<timeline::Sequence> newSequence, utility::ErrorState& errorState)
	{
		// retrieve current sequence and element
		double time = mProps.mCurrentTimeOfMouseInSequence;
		timeline::Sequence* sequence = mSequencePlayer->getSequenceAtTime(time);
		timeline::SequenceElement* element = sequence->getElementAtTime(time);

		// make sure curves get re-evaluated
		for (auto& pair : mProps.mDirtyFlags)
		{
			pair.second = true;
		}

		// check if sequence is not to close to other sequence already existing..
		for (const auto* sequence : mSequencePlayer->getSequences())
		{
			float diff = math::abs((float)time - (float)sequence->getStartTime());
			if (!errorState.check(
				diff > 0.1f,
				"New sequence to close to other sequence existing [%s] ", 
				sequence->mName.c_str())) // to close
			{
				return false;
			}
		}

		// set new duration of current element and calculate new duration of new element
		float newDuration = time - element->getStartTime();
		float deltaDuration = element->mDuration - newDuration;
		element->mDuration = newDuration;

		// configure new sequence
		std::string timeString = getTimeString();
		newSequence->mID = "GeneratedSequence" + timeString;
		newSequence->mName = newSequence->mID;
		newSequence->mIndexInSequenceContainer = sequence->mIndexInSequenceContainer;
		newSequence->setStartTime(time);

		// make new element and configure
		std::unique_ptr<flexblock::FlexblockSequenceTransition> newElement = std::make_unique<flexblock::FlexblockSequenceTransition>();
		newElement->mID = "GeneratedElement" + timeString;
		newElement->mDuration = deltaDuration;

		if (!newElement->init(errorState))
		{
			return false;
		}
		else
		{
			// insert and move ownership
			newSequence->insertElement(std::move(newElement));

			if (!newSequence->init(errorState))
			{
				return false;
			}
			else
			{
				// delete elements in current sequence after inserted sequence
				int index = -1;
				for (int i = 0; i < sequence->getElements().size(); i++)
				{
					if (element == sequence->getElements()[i])
					{
						index = i;
						break;
					}
				}

				if (index >= 0 && index < sequence->getElements().size() - 1)
				{
					sequence->eraseElements(index + 1, sequence->getElements().size());
				}

				// insert the new sequence and move ownership
				mSequencePlayer->insertSequence(std::move(newSequence));
			}
		}

		return true;
	}


	std::string FlexblockGui::getTimeString()
	{
		time_t rawtime;
		struct tm * timeinfo;
		char buffer[80];

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		strftime(buffer, sizeof(buffer), "%d-%m-%Y%H:%M:%S", timeinfo);
		return std::string(buffer);
	}

	
	void FlexblockGui::showMotorSteps()
	{
		ImGui::Begin("MotorSteps");

		const auto& motorSteps = mFlexBlock->getMotorSteps();

		for (int i = 0; i < motorSteps.size(); i++)
		{
			ImGui::Text("%i : %.3f meter / %.0f steps", i+1, motorSteps[i] / mFlexBlock->getMotorStepsPerMeter(), motorSteps[i]);
		}

		ImGui::End();
	}


	void FlexblockGui::showMotorControlWindow()
	{
		static auto showTip = [this](const char* tip)
		{
			if (mProps.mShowToolTips)
			{
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text(tip);
					ImGui::EndTooltip();
				}
			}
		};

		static bool firstTime = true;

		static float velocity = mMotorController->mVelocity;
		static float acceleration = mMotorController->mAcceleration;
		static float torque = mMotorController->mTorque;
		static float maxVelocity = mMotorController->mMaxVelocity;

		RGBColorFloat text_color = mTextColor.convert<RGBColorFloat>();
		ImGui::Begin("Motor Controls");
		if (ImGui::Button("Reset Position"))
		{
			ImGui::OpenPopup("Reset Position Confirmation");
		}
		showTip("Resets all motor positions to their current position.\n This means their current position will be the new 0.0 position.");

		mProps.mEnableFlexblock = mFlexBlock->getEnableMotorController();
		if (ImGui::Checkbox("Enable flexblock", &mProps.mEnableFlexblock))
		{
			mFlexBlock->setEnableMotorController(mProps.mEnableFlexblock);
		}
		showTip("Enables the flexblock algorithm to control the motors");

		ImGui::SameLine();
		ImGui::Checkbox("Advanced", &mProps.mAdvancedMotorInterface);
		showTip("Opens advanced motor control");
		ImGui::SameLine();
		if (ImGui::Checkbox("Calibration Mode", &mProps.mCalibrationMode))
		{
			if (mProps.mCalibrationMode)
			{
				mMotorController->mVelocity = mMotorController->mCalibrationVelocity;
				mMotorController->mAcceleration = mMotorController->mCalibrationAcceleration;
				mMotorController->mTorque = mMotorController->mCalibrationTorque;
				mMotorController->mMaxVelocity = mMotorController->mCalibrationMaxVelocity;
			}
			else
			{
				mMotorController->mVelocity = velocity;
				mMotorController->mAcceleration = acceleration;
				mMotorController->mTorque = torque;
				mMotorController->mMaxVelocity = maxVelocity;
			}
		}
		showTip("Calibration mode means that the Torque, Acceleration and Velocity calibration values of the MACController will be used");

		ImGui::SameLine();
		if (mProps.mCalibrationMode)
		{
			if (ImGui::Checkbox("Clamp Meters", &mProps.mClampMetersInCalibrationMode))
			{

			}
			showTip("When clamped, meters cannot go lower then zero");
		}
		

		ImGui::Separator();
		std::vector<MacPosition> position_data;
		mMotorController->copyPositionData(position_data);
		for (int i = 0; i < mMotorController->getSlaveCount(); i++)
		{
			if (ImGui::CollapsingHeader(utility::stringFormat("motor: %d mapping %d", i + 1, mFlexBlock->getMotorMapping()[i] + 1).c_str()))
			{
				const double counts = mFlexBlock->getMotorStepsPerMeter();
				float current_meters = (float)(((double)mMotorController->getActualPosition(i)) / counts);

				if (firstTime)
				{
					mTargetMeters[i] = current_meters;
				}
				ImGui::Text("Current Motor Mode: %s", mMotorController->modeToString(mMotorController->getActualMode(i)).c_str());
				ImGui::Text("Current Motor Position: %d", mMotorController->getActualPosition(i));
				ImGui::Text("Current Motor Meters: %.3f", current_meters);
				ImGui::Text("Current Motor Velocity: %.1f / velocity %s / max velocity %s", mMotorController->getActualVelocity(i), std::to_string(mMotorController->mVelocity).c_str(), std::to_string(mMotorController->mMaxVelocity).c_str());
				ImGui::Text("Current Motor Torque: %.1f / max Torque %s", mMotorController->getActualTorque(i), std::to_string(mMotorController->mTorque).c_str());
				ImGui::Text("Current Acceleration : %s", std::to_string(mMotorController->mAcceleration).c_str());
				ImGui::Text("Target Meters: %.3f", mTargetMeters[i]);
				ImGui::PushID(i);

				int req_pos = static_cast<int>(position_data[i].mTargetPosition);
				if (mProps.mAdvancedMotorInterface)
				{
					bool dig_pin = position_data[i].getDigitalPin(0);
					if (ImGui::Checkbox("Digital Pin", &dig_pin))
					{
						position_data[i].setDigitalPin(0, dig_pin);
					}

					if (ImGui::InputInt("Position", &req_pos, 1, 50))
					{
						position_data[i].setTargetPosition(req_pos);
						mMotorController->setPositionData(position_data);
					}

					int req_vel = mMotorController->getVelocity(i);
					if (ImGui::InputInt("Velocity", &req_vel, 1, 10))
					{
						req_vel = static_cast<int>(math::clamp<float>(static_cast<float>(req_vel), 0.0f, mMotorController->mMaxVelocity));
						mMotorController->setVelocity(i, static_cast<float>(req_vel));
					}

					int req_tor = mMotorController->mTorque;
					if (ImGui::InputInt("Torque", &req_tor, 1, 5))
					{
						req_tor = math::clamp<int>(req_tor, 0, 300);
						mMotorController->setTorque(i, static_cast<float>(req_tor));
					}
				
					int req_acc = mMotorController->mAcceleration;
					if (ImGui::InputInt("Acceleration", &req_acc, 1, 10))
					{
						req_acc = static_cast<int>(math::clamp<float>(static_cast<float>(req_acc), 0.0f, mMotorController->mMaxVelocity));
						mMotorController->setAcceleration(i, req_acc);
						mMotorController->mAcceleration = req_acc;
					}

					// meters
					//  129473,415472573 = 1 meter
					float target_meter = mTargetMeters[i];
					if (ImGui::InputFloat("Target meters", &target_meter, 0.001f, 0.001f, 3))
					{
						int32 newCounts = (int32)((double)target_meter * counts);

						position_data[i].setTargetPosition(newCounts);
						mMotorController->setPositionData(position_data);
						mTargetMeters[i] = target_meter;
					}
				}else if (!mProps.mAdvancedMotorInterface)
				{
					float target_meter = mTargetMeters[i];
					if (ImGui::Button("Give Meter"))
					{
						target_meter = current_meters + 1.0f;
						int32 newCounts = (int32)((double)target_meter * counts);

						position_data[i].setTargetPosition(newCounts);
						mMotorController->setPositionData(position_data);
						mTargetMeters[i] = target_meter;
					}
					ImGui::SameLine();
					if (ImGui::Button("Take Meter"))
					{
						target_meter = current_meters - 1;
						int32 newCounts = (int32)((double)target_meter * counts);
						position_data[i].setTargetPosition(newCounts);
						mMotorController->setPositionData(position_data);
						mTargetMeters[i] = target_meter;
					}

					if (ImGui::Button("Give Decimeter"))
					{
						target_meter = current_meters + 0.1f;
						int32 newCounts = (int32)((double)target_meter * counts);
						position_data[i].setTargetPosition(newCounts);
						mMotorController->setPositionData(position_data);
						mTargetMeters[i] = target_meter;
					}
					ImGui::SameLine();
					if (ImGui::Button("Take Decimeter"))
					{
						target_meter = current_meters - 0.1f;
						int32 newCounts = (int32)((double)target_meter * counts);
						position_data[i].setTargetPosition(newCounts);
						mMotorController->setPositionData(position_data);
						mTargetMeters[i] = target_meter;
					}
					
					if (ImGui::Button("Give Centimeter"))
					{
						target_meter = current_meters + 0.01f;
						int32 newCounts = (int32)((double)target_meter * counts);
						position_data[i].setTargetPosition(newCounts);
						mMotorController->setPositionData(position_data);
						mTargetMeters[i] = target_meter;
					}
					ImGui::SameLine();
					if (ImGui::Button("Take Centimeter"))
					{
						target_meter = current_meters - 0.01f;
						int32 newCounts = (int32)((double)target_meter * counts);
						position_data[i].setTargetPosition(newCounts);
						mMotorController->setPositionData(position_data);
						mTargetMeters[i] = target_meter;
					}
					
					if (ImGui::Button("Give Millimeter"))
					{
						target_meter = current_meters + 0.001f;
						int32 newCounts = (int32)((double)target_meter * counts);
						position_data[i].setTargetPosition(newCounts);
						mMotorController->setPositionData(position_data);
						mTargetMeters[i] = target_meter;
					}
					ImGui::SameLine();
					if (ImGui::Button("Take Millimeter"))
					{
						target_meter = current_meters - 0.001f;
						int32 newCounts = (int32)((double)target_meter * counts);
						position_data[i].setTargetPosition(newCounts);
						mMotorController->setPositionData(position_data);
						mTargetMeters[i] = target_meter;
					}
				}

				// clamp targetmeters
				if (mProps.mCalibrationMode && mProps.mClampMetersInCalibrationMode)
				{
					if (mTargetMeters[i] < 0.0f)
						mTargetMeters[i] = 0.0f;
				}

				bool error = false;
				if (mMotorController->hasError(i))
				{
					if (ImGui::Button("Clear Errors"))
						mMotorController->clearErrors(i);

					std::vector<MACController::EErrorStat> errors;
					mMotorController->getErrors(i, errors);
					for (const auto& error : errors)
					{
						ImGui::TextColored(text_color, MACController::errorToString(error).c_str());
					}
					error = true;
				}

				if (!mMotorController->isOnline(i))
				{
					ImGui::TextColored(text_color, "Slave Lost!");
					error = true;
				}

				ImGui::Separator();
				ImGui::PopID();
				if (!error)
				{
					ImGui::Text("No Errors");
				}
			}
		}
		if (mMotorController->getSlaveCount() == 0)
			ImGui::Text("No slaves found");

		ImGui::PushStyleColor(0, colorRed);
		if (ImGui::Button("!STOP!"))
		{
			mMotorController->emergencyStop();
		}
		showTip("Stops all motors");
		if (ImGui::Button("!START!"))
		{
			utility::ErrorState errorState;
			mMotorController->start(errorState);
		}
		showTip("Starts all motors");
		ImGui::PopStyleColor();

		ImGui::Separator();
		ImGui::Text("Errors : ");
		std::vector<std::string> errorStrings;
		for (int i = 0; i < mMotorController->getSlaveCount(); i++)
		{
			std::vector<MACController::EErrorStat> macErrors;
			mMotorController->getErrors(i, macErrors);
			if (macErrors.size() > 0)
			{
				for (int j = 0; j < macErrors.size(); j++)
				{
					errorStrings.emplace_back("Motor : " + std::to_string(i) + " " +MACController::errorToString(macErrors[j]));
				}
			}
		}
		if (errorStrings.size() > 0)
		{
			for (int i = 0; i < errorStrings.size(); i++)
			{
				ImGui::TextColored(text_color, errorStrings[i].c_str());
			}
		}
		else
		{
			ImGui::Text("No Errors!");
		}

		if (ImGui::BeginPopup("Reset Position Confirmation"))
		{
			ImGui::Text("Are you sure you want to reset the postion of all motors ?");
			if (ImGui::Button("Reset"))
			{
				utility::ErrorState error;
				mMotorController->resetPosition(0, error);

				for (float& meter : mTargetMeters)
				{
					meter = 0.0f;
				}
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::End();

		firstTime = false;
	}
}