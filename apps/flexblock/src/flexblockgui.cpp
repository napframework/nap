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
		ObjectPtr<ParameterFloat> parameter = resourceManager->findObject<ParameterFloat>("Slack");

		assert(parameter != nullptr);

		mParameters.emplace_back(parameter.get());
		parameter->setValue(0.0f);

		parameter->valueChanged.connect([this](float newValue)
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
	}


	void FlexblockGui::updateInput(int index, float value)
	{
		mFlexBlock->setMotorInput(index, value);
	}


	void FlexblockGui::update(double deltaTime)
	{
		mTime += deltaTime;

		if (mHide)
			return;

		ImGui::SetNextWindowPos(ImVec2(10, mWindowSize.y * 0.5f + 10));
		ImGui::SetNextWindowSize(ImVec2(mWindowSize.x * 0.5f - 20, mWindowSize.y * 0.4f - 20));
		mParameterGUI->show(mParameterService.hasRootGroup() ? &mParameterService.getRootGroup() : nullptr);

		showInfoWindow();

		showTimeLineWindow();

		if (mProps.mShowPlaylist)
		{
			showPlaylist();
		}

		showMotorControlWindow();
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
		//
		ImGui::SetNextWindowPos(ImVec2(10, 10));
		//ImGui::SetNextWindowSize(ImVec2(mWindowSize.x - 20, mWindowSize.y * 0.5f - 20));

		// set next window content size to timeline ( child ) width to make scroll bar fit
		ImGui::SetNextWindowContentSize(
			ImVec2(mProps.mChildWidth + 100.0f, mProps.mChildHeight + 200.0f ));

		// begin the window
		ImGui::Begin("Timeline", 0, ImGuiWindowFlags_HorizontalScrollbar );

		bool needToOpenPopup = false;
		std::string popupIdToOpen = "";

		// draw timeline
		drawTimeline(needToOpenPopup, popupIdToOpen);

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
		handleEditMotorvaluePopup();

		// release mouse
		if (ImGui::IsMouseReleased(0) && !mProps.mInPopup)
		{
			mProps.mSelectedElement = nullptr;
			mProps.mTangentPtr = nullptr;
			mProps.mCurvePtr = nullptr;
			mProps.mSelectedSequence = nullptr;
			mProps.mCurrentSelectedMotor = 0;

			mProps.mCurrentAction = TimeLineActions::NONE;
		}

		ImGui::End();
	}


	void FlexblockGui::drawTimeline(bool& outPopupOpened, std::string& outPopupId)
	{
		// we need to redraw the curves again after the first time drawing the timeline
		// this is because the window gets resized after drawing the timeline for the first time
		static bool firstTime = true;

		float windowWidth = ImGui::GetWindowWidth();
		float scrollX = ImGui::GetScrollX();

		if (scrollX != mProps.mPrevScrollX)
		{
			mProps.mDirty = true;
			mProps.mPrevScrollX = scrollX;
		}

		if (ImGui::GetScrollY() != mProps.mPrevScrollY)
		{
			mProps.mPrevScrollY = ImGui::GetScrollY();
			mProps.mDirty = true;
		}

		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 50));

		// begin timeline child
		ImGui::BeginChild("", ImVec2(mProps.mChildWidth + 32, mProps.mChildHeight), false, ImGuiWindowFlags_NoMove);
		{
			float timeInDisplayStart = ( math::max(0.0f, scrollX - 32 ) / mProps.mChildWidth) * mSequencePlayer->getDuration();
			float timeInDisplayEnd = timeInDisplayStart + (windowWidth / mProps.mChildWidth) * mSequencePlayer->getDuration();

			//printf("%f %f \n", timeInDisplayStart, timeInDisplayEnd);

			ImDrawList* drawList = ImGui::GetWindowDrawList();

			//
			const ImVec2 childSize = ImVec2(mProps.mChildWidth, mProps.mChildHeight - 125.0f);

			// start top left with a little bit of margin
			mProps.mTopLeftPosition = ImGui::GetCursorScreenPos();
			mProps.mTopLeftPosition.x += 30;
			mProps.mTopLeftPosition.y += 20;
			ImVec2 bottomRightPos = ImVec2(
				mProps.mTopLeftPosition.x + childSize.x, 
				mProps.mTopLeftPosition.y + childSize.y);

			// timeline background
			drawList->AddRectFilled(mProps.mTopLeftPosition, bottomRightPos,
				colorBlack, 1.0f);

			// motors backgrounds
			for (int l = 0; l < 9; l++)
			{
				drawList->AddRect(
					ImVec2(mProps.mTopLeftPosition.x, math::lerp<float>(mProps.mTopLeftPosition.y, bottomRightPos.y, (float)l / 9.0f)),
					ImVec2(bottomRightPos.x, math::lerp<float>(mProps.mTopLeftPosition.y, bottomRightPos.y, (float)(l + 1) / 9.0f)),
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
						colorWhite);

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
						colorWhite);

					// rename sequence action
					if (mProps.mCurrentAction == TimeLineActions::NONE)
					{
						if (ImGui::IsMouseHoveringRect(sequenceBoxUpperLeft, sequenceBoxLowerRight))
						{
							drawList->AddRectFilled(
								sequenceBoxUpperLeft,
								sequenceBoxLowerRight,
								colorWhite);

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
							float elementPos = (element->getStartTime() - sequences[i]->getStartTime()) / sequences[i]->getDuration();
							float elementWidth = element->mDuration / sequences[i]->getDuration();

							// the bottom line
							drawList->AddLine(
								ImVec2(mProps.mTopLeftPosition.x + startX + width * elementPos, bottomRightPos.y),
								ImVec2(mProps.mTopLeftPosition.x + startX + width * elementPos, bottomRightPos.y + 50),
								colorLightGrey);

							// line in timeline
							drawList->AddLine(
								ImVec2(mProps.mTopLeftPosition.x + startX + width * elementPos, mProps.mTopLeftPosition.y),
								ImVec2(mProps.mTopLeftPosition.x + startX + width * elementPos, bottomRightPos.y + 50),
								colorLightGrey);

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
											mProps.mDirty = true;
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
									mProps.mDirty = true;

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
								float motor_height = childSize.y / 9.0f;
								for (int m = 0; m < 9; m++)
								{
									if ((mProps.mMotorHandlerIndexMask >> m) & 1UL)
									{
										const float circleSize = 6.0f;

										bool filled = false;
										float x = mProps.mTopLeftPosition.x + startX + width * elementPos + width * elementWidth;
										float value = static_cast<ParameterFloat*>(element->getEndParameters()[m])->mValue;
										float y = (bottomRightPos.y - motor_height * (float)m) - motor_height * value;

										// handle dragging of motor values
										if (mProps.mCurrentAction == TimeLineActions::NONE)
										{
											if (ImGui::IsMouseHoveringRect(ImVec2(x - 12, y - 12), ImVec2(x + 12, y + 12)))
											{
												filled = true;
												if (ImGui::IsMouseClicked(0))
												{
													mProps.mCurrentAction = TimeLineActions::DRAGGING_MOTORVALUE;
													mProps.mCurrentSelectedMotor = m;
													mProps.mSelectedElement = element;
													mProps.mDirty = true;
												}
												else if (ImGui::IsMouseClicked(1))
												{
													mProps.mCurrentAction = TimeLineActions::EDIT_MOTORVALUE_POPUP;
													mProps.mCurrentSelectedMotor = m;
													mProps.mSelectedElement = element;
													mProps.mDirty = true;
													mProps.mInPopup = true;
													outPopupOpened = true;
													outPopupId = "Edit Motorvalue";
												}
												else
												{
													ImGui::BeginTooltip();
													ImGui::Text("Hold left mouse button to drag \nPress right mouse button to edit value");
													ImGui::EndTooltip();
												}
											}
										}

										if (mProps.mCurrentAction == TimeLineActions::DRAGGING_MOTORVALUE
											&& ImGui::IsMouseDragging() &&
											m == mProps.mCurrentSelectedMotor &&
											element == mProps.mSelectedElement)
										{
											filled = true;
											mProps.mDirty = true;

											ImVec2 mousePos = ImGui::GetMousePos();

											float adjust = (mousePos.y - y) / motor_height;

											ParameterFloat* parameterFloat = static_cast<ParameterFloat*>(element->getEndParameters()[m]);
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
								float motor_height = childSize.y / 9.0f;
								for (int m = 0; m < 9; m++)
								{
									if ((mProps.mMotorHandlerIndexMask >> m) & 1UL)
									{
										//
										mProps.mDirty = true;

										//
										ParameterFloat* startParameterFloat = static_cast<ParameterFloat*>(element->getStartParameters()[m]);
										ParameterFloat* endParameterFloat = static_cast<ParameterFloat*>(element->getEndParameters()[m]);

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

											mProps.mDirty = true;

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
												auto newPoint = curves[m]->mPoints[0];
												newPoint.mPos.mTime = pX;
												newPoint.mPos.mValue = math::clamp(pY, startValue, endValue);

												// add the point to the curve
												curves[m]->mPoints.emplace_back(newPoint);

												// update curve
												curves[m]->invalidate();
											}
										}

										// vector stores deletion of points
										std::vector<int> pointsToDelete;

										// iterate trough the points of the curve
										for (int p = 0; p < curves[m]->mPoints.size(); p++)
										{
											// get a reference
											auto& point = curves[m]->mPoints[p];

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

													mProps.mDirty = true;
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
											
												mProps.mDirty = true;
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

													mProps.mDirty = true;
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

												mProps.mDirty = true;
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

													mProps.mDirty = true;
												}
											}

											// handle dragging
											if (mProps.mCurrentAction == TimeLineActions::DRAGGING_TANGENT &&
												mProps.mTangentPtr == &point.mInTan)
											{
												ImVec2 mousePos = ImGui::GetMousePos();

												float adjust = (mousePos.y - y) / motor_height;
												mProps.mTangentPtr->mValue -= adjust * range ;

												adjust = (mousePos.x - x) / elementSizeWidth;
												mProps.mTangentPtr->mTime += adjust;

												mProps.mDirty = true;
											}

											// make sure tangents are linked
											if (mProps.mCurrentAction == TimeLineActions::DRAGGING_TANGENT &&
												mProps.mTangentPtr == &point.mOutTan)
											{
												point.mInTan.mTime = -mProps.mTangentPtr->mTime;
												point.mInTan.mValue = -mProps.mTangentPtr->mValue;

												curves[m]->invalidate();
												mSequencePlayer->reconstruct();
											}

											if (mProps.mCurrentAction == TimeLineActions::DRAGGING_TANGENT &&
												mProps.mTangentPtr == &point.mInTan)
											{
												point.mOutTan.mTime = -mProps.mTangentPtr->mTime;
												point.mOutTan.mValue = -mProps.mTangentPtr->mValue;

												curves[m]->invalidate();
												mSequencePlayer->reconstruct();
											}
										}

										// delete any points
										for (const int index : pointsToDelete)
										{
											curves[m]->mPoints.erase(curves[m]->mPoints.begin() + index);
											curves[m]->invalidate();
										}
									}
								}
							}
						}
					}

					// draw motor inputs 
					if (mProps.mDirty)
					{
						//
						mProps.mCachedCurve.clear();

						mProps.mCachedCurve = std::vector<std::vector<ImVec2>>(9);

						// create parameters that we evaluate
						std::vector<std::unique_ptr<ParameterFloat>> parametersPts;
						std::vector<Parameter*> parameters;
						for (int p = 0; p < 9; p++)
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
							for (int l = 0; l < 9; l++)
							{
								float yPart = childSize.y / 9.0f;
								float yStart = yPart * l;

								mProps.mCachedCurve[l].emplace_back(ImVec2(
									partStart * mProps.mChildWidth + mProps.mTopLeftPosition.x + childSize.x * part * (p * (1.0f / (float)steps)),
									bottomRightPos.y - yStart - yPart * static_cast<ParameterFloat*>(parameters[l])->mValue));
							}
						}

						mProps.mDirty = false;
					}
			
					if (mProps.mCachedCurve.size() == 9)
					{
						for (int l = 0; l < 9; l++)
						{
							if (mProps.mCachedCurve[l].size() > 0)
							{
								// draw the polylines 
								drawList->AddPolyline(
									&*mProps.mCachedCurve[l].begin(),
									mProps.mCachedCurve[l].size(),
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
			for (int i = 0; i < 9; i++)
			{
				float y_pos = (childSize.y / 9) * i + 4;
				// draw motor text
				drawList->AddText(
					ImVec2(mProps.mTopLeftPosition.x - 25, mProps.mTopLeftPosition.y + y_pos),
					colorWhite,
					i == 0 ? "S" : std::to_string(9 - i).c_str());

				//
				const ImVec2 rectTopLeft(mProps.mTopLeftPosition.x - 15, mProps.mTopLeftPosition.y + y_pos + 5);
				const ImVec2 rectBotRight(mProps.mTopLeftPosition.x - 5, mProps.mTopLeftPosition.y + y_pos + 15);

				bool filled = false;
				int motorId = 8 - i;
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
							if ( (mProps.mMotorHandlerIndexMask >> motorId ) & 1UL )
							{
								mProps.mMotorHandlerIndexMask &= ~(1UL << motorId);
							}
							else
							{
								mProps.mMotorHandlerIndexMask |= 1UL << motorId;
							}
							mProps.mCurrentAction = TimeLineActions::ENABLING_HANDLERS;
						}
					}
				}

				if (!filled)
					filled = (mProps.mMotorHandlerIndexMask >> motorId) & 1UL;

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
			if (mProps.mDrawMouseCursorInTimeline )
			{
				drawList->AddLine(
					ImVec2(mProps.mTopLeftPosition.x + mProps.mMouseCursorPositionInTimeline, mProps.mTopLeftPosition.y),
					ImVec2(mProps.mTopLeftPosition.x + mProps.mMouseCursorPositionInTimeline, bottomRightPos.y),
					colorRed, 1.5f);
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

			ImGui::Spacing();
			ImGui::Spacing();
		}

		// make sure the curves get cached again after they are drawn the first time
		if (firstTime)
		{
			firstTime = false;
			mProps.mDirty = true;
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
			mProps.mDirty = true;
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
			mProps.mDirty = true;

		ImGui::PopItemWidth();

		showTip("Vertical zoom");

		// zoom of timeline
		ImGui::SameLine();
		ImGui::PushItemWidth(100.0f);
		if (ImGui::SliderFloat("Horizontal Zoom", &mProps.mLengthPerSecond, 4.0f, 60.0f, ""))
			mProps.mDirty = true;
		ImGui::PopItemWidth();

		mProps.mChildWidth = (mSequencePlayer->getDuration() / mProps.mLengthPerSecond) * ImGui::GetWindowWidth();
		
		showTip("Horizontal zoom");

		// curves resolution
		ImGui::SameLine();
		ImGui::PushItemWidth(100.0f);
		if (ImGui::SliderInt("Curve Res.", &mProps.mCurveResolution, 50, 666, ""))
			mProps.mDirty = true;
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
			mProps.mDirty = true;
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
					mProps.mDirty = true;

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

		std::stringstream stringStream;

		stringStream << std::setw(2) << std::setfill('0') << seconds;
		std::string secondsString = stringStream.str();

		stringStream = std::stringstream();
		stringStream << std::setw(2) << std::setfill('0') << minutes;
		std::string minutesString = stringStream.str();

		std::string hoursString = "";
		if (hours > 0)
		{
			stringStream = std::stringstream();
			stringStream << std::setw(2) << std::setfill('0') << hours;
			hoursString = stringStream.str() + ":";
		}

		return hoursString + minutesString + ":" + secondsString;
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
				mProps.mDirty = true;
			}

			// enable delete if sequence has more then one element
			if (owningSequence != nullptr &&
				owningSequence->getElements().size() > 1)
			{
				if (ImGui::Button("Delete"))
				{
					mSequencePlayer->removeSequenceElement(owningSequence, mProps.mSelectedElement);
					mProps.mDirty = true;

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
		ImGui::SetNextWindowPos(ImVec2(10, mWindowSize.y * 0.9f + 10));
		ImGui::SetNextWindowSize(ImVec2(mWindowSize.x * 0.5f - 20, mWindowSize.y * 0.1f - 20));

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


	void FlexblockGui::handleEditMotorvaluePopup()
	{
		if (ImGui::BeginPopup("Edit Motorvalue"))
		{
			ImGui::Text("Edit motorvalue");

			if (mProps.mSelectedElement != nullptr && 
				mProps.mCurrentSelectedMotor > -1 && 
				mProps.mCurrentSelectedMotor < 9)
			{
				ParameterFloat* parameterFloat =
					dynamic_cast<ParameterFloat*>(mProps.mSelectedElement->getEndParameters()[mProps.mCurrentSelectedMotor]);

				if (parameterFloat != nullptr)
				{
					float value = parameterFloat->mValue;
					ImGui::InputFloat("", &value, 0.001f, 0.01f, 3);
					value = math::clamp(value, 0.0f, 1.0f);
					parameterFloat->setValue(value);
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
			if (mProps.mCurrentAction == TimeLineActions::EDIT_MOTORVALUE_POPUP)
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
		mProps.mDirty = true;

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

				for (int i = 0; i < previousElement->mEndParameterResourcePtrs.size(); i++)
				{
					previousElement->mEndParameterResourcePtrs[i]->setValue(*mParameters[i]);
				}
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
		mProps.mDirty = true;

		// check if sequence is not to close to other sequence already existing..
		for (const auto* sequence : mSequencePlayer->getSequences())
		{
			float diff = math::abs((float)time - (float)sequence->getStartTime());
			if (!errorState.check(
				diff > 0.1f,
				"New sequence to close to other sequence existing [%s] ", sequence->mName.c_str())) // to close
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


	void FlexblockGui::showMotorControlWindow()
	{
		RGBColorFloat text_color = mTextColor.convert<RGBColorFloat>();
		ImGui::Begin("Motor Controls");
		ImGui::SliderInt("Reset Value", &mResetMotorPos, -5000000, 5000000);
		ImGui::SameLine();
		if (ImGui::Button("Reset Position"))
		{
			utility::ErrorState error;
			mMotorController->resetPosition(mResetMotorPos, error);
		}

		ImGui::Separator();
		for (int i = 0; i < mMotorController->getSlaveCount(); i++)
		{
			if (ImGui::CollapsingHeader(utility::stringFormat("motor: %d", i + 1).c_str()))
			{
				ImGui::Text("Curent Motor Mode: %s", mMotorController->modeToString(mMotorController->getActualMode(i)).c_str());
				ImGui::Text("Current Motor Position: %d", mMotorController->getActualPosition(i));
				ImGui::Text("Current Motor Velocity: %.1f", mMotorController->getActualVelocity(i));
				ImGui::Text("Current Motor Torque: %.1f", mMotorController->getActualTorque(i));
				int req_pos = static_cast<int>(mMotorController->getPosition(i));
				if (ImGui::SliderInt("Position", &req_pos, -5000000, 5000000))
				{
					mMotorController->setPosition(i, req_pos);
				}

				int req_vel = mMotorController->getVelocity(i);
				if (ImGui::SliderInt("Velocity", &req_vel, 0, mMotorController->mMaxVelocity))
				{
					mMotorController->setVelocity(i, static_cast<float>(req_vel));
				}
				ImGui::Separator();
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
		ImGui::PopStyleColor();

		ImGui::End();
	}
}