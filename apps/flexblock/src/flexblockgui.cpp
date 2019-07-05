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
	/**
	 * Imgui statics
	 */
	static bool showInfo = false;
	static float lengthPerSecond = 60.0f;
	static float childWidth = 1000.0f;
	static float child_height = 350.0f;
	static bool followPlayer = false;
	static int totalCurveResolution = 75;
	static timeline::SequenceElement* selectedElement = nullptr;
	static float beginPos = 0.0f;
	static int motorDragged = 0;
	static int selectedShowIndex = 0;
	static bool inPopup = false;
	static int enableMotorHandlerIndexBitMask = 0;
	static float currentTimeOfMouseInSequence = 0.0f;
	static std::string errorString;
	static timeline::Sequence* selectedSequence = nullptr;
	static bool showToolTips = true;
	static ImVec2 topLeft;
	static bool drawCursor = false;
	static float cursorPos = 0.0f;
	static bool dirty = true;
	static std::vector<std::vector<ImVec2>> cachedCurve;
	static float prevScrollX = 0.0f;
	static float prevScrollY = 0.0f;

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

	enum TimeLineActions
	{
		NONE = 0,
		DRAGGING_ELEMENT = 1,
		DRAGGING_CURVEPOINT = 2,
		DRAGGING_TANGENT = 3,
		DRAGGING_MOTORVALUE = 4,
		ADD_CURVEPOINT = 5,
		DELETE_CURVEPOINT = 6,
		DRAGGING_PLAYERPOSITION = 7,
		ELEMENT_RENAME_POPUP = 8,
		ENABLING_HANDLERS = 9,
		SEQUENCE_RENAME_POPUP = 10,
		INSERTION_POPUP = 11,
		SAVE_POPUP = 12,
		LOAD_POPUP = 13
	};

	static TimeLineActions currentTimelineAction = TimeLineActions::NONE;

	math::FCurvePoint<float, float> *curvePointPtr = nullptr;
	math::FComplex<float, float> *tangentPtr = nullptr;

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

					if (adressParts.size() == 3)
					{
						if (adressParts[1] == "flexblock")
						{
							int parameter = std::stoi(adressParts[2]) - 1;
							if (parameter >= 0 && parameter < 8)
							{
								float value = message.getArgument(0)->asFloat();
								mParameters[parameter]->setValue(value);
							}
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

		if (mShowPlaylist)
		{
			showPlaylist();
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


	void FlexblockGui::showTimeLineWindow()
	{
		//
		ImGui::SetNextWindowPos(ImVec2(10, 10));
		//ImGui::SetNextWindowSize(ImVec2(mWindowSize.x - 20, mWindowSize.y * 0.5f - 20));

		// set next window content size to timeline ( child ) width to make scroll bar fit
		ImGui::SetNextWindowContentSize(ImVec2(childWidth + 100.0f, child_height + 200.0f ));

		// begin the window
		ImGui::Begin("Timeline", 0, ImGuiWindowFlags_HorizontalScrollbar );

		bool needToOpenPopup = false;
		std::string popupIdToOpen = "";

		// draw player controls
		drawTimelinePlayerControls(needToOpenPopup, popupIdToOpen);

		// draw timeline
		drawTimeline(needToOpenPopup, popupIdToOpen);

		//
		if (needToOpenPopup)
			ImGui::OpenPopup(popupIdToOpen.c_str());

		//
		handleLoadPopup();

		//
		handleSaveAsPopup();

		// 
		handleSequenceActionsPopup();

		//
		handleElementActionsPopup();

		// insertion popup
		handleInsertionPopup();

		// release mouse
		if (ImGui::IsMouseReleased(0) && !inPopup)
		{
			selectedElement = nullptr;
			tangentPtr = nullptr;
			curvePointPtr = nullptr;
			selectedSequence = nullptr;
			motorDragged = 0;

			currentTimelineAction = TimeLineActions::NONE;
		}

		ImGui::End();
	}


	void FlexblockGui::drawTimeline(bool& outPopupOpened, std::string& outPopupId)
	{
		float windowWidth = ImGui::GetWindowWidth();
		float scrollX = ImGui::GetScrollX();

		if (scrollX != prevScrollX)
		{
			dirty = true;
			prevScrollX = scrollX;
		}

		if (ImGui::GetScrollY() != prevScrollY)
		{
			prevScrollY = ImGui::GetScrollY();
			dirty = true;
		}

		// begin timeline child
		ImGui::BeginChild("", ImVec2(childWidth + 32, child_height), false, ImGuiWindowFlags_NoMove);
		{
			float timeInDisplayStart = ( math::max(0.0f, scrollX - 32 ) / childWidth) * mSequencePlayer->getDuration();
			float timeInDisplayEnd = timeInDisplayStart + (windowWidth / childWidth) * mSequencePlayer->getDuration();

			//printf("%f %f \n", timeInDisplayStart, timeInDisplayEnd);

			ImDrawList* drawList = ImGui::GetWindowDrawList();

			//
			const ImVec2 childSize = ImVec2(childWidth, child_height - 125.0f);

			// start top left with a little bit of margin
			topLeft = ImGui::GetCursorScreenPos();
			topLeft.x += 30;
			topLeft.y += 20;
			ImVec2 bottomRightPos = ImVec2(topLeft.x + childSize.x, topLeft.y + childSize.y);

			// timeline background
			drawList->AddRectFilled(topLeft, bottomRightPos,
				colorBlack, 1.0f);

			// motors backgrounds
			for (int l = 0; l < 8; l++)
			{
				drawList->AddRect(
					ImVec2(topLeft.x, math::lerp<float>(topLeft.y, bottomRightPos.y, (float)l / 8.0f)),
					ImVec2(bottomRightPos.x, math::lerp<float>(topLeft.y, bottomRightPos.y, (float)(l + 1) / 8.0f)),
					colorWhite, 1.0f);
			}

			// right border
			drawList->AddLine(
				ImVec2(topLeft.x + childSize.x, topLeft.y),
				ImVec2(topLeft.x + childSize.x, bottomRightPos.y),
				colorWhite);

			// draw timesteps on top of timeline
			const int timeStamps = 10;
			float timePart = (timeInDisplayEnd - timeInDisplayStart) / timeStamps;
			for (int i = 1; i < timeStamps; i++)
			{
				float time = (timeInDisplayStart + i * timePart);
				ImVec2 start(
					topLeft.x + (time / mSequencePlayer->getDuration()) * childWidth,
					topLeft.y);
				ImVec2 end = start;
				end.y -= 10;

				drawList->AddLine(start, end, colorWhite, 2.0f);
				
				ImVec2 textPos = end;
				textPos.x += 3;
				textPos.y -= 6;
				drawList->AddText(textPos, colorWhite, formatTimeString(time).c_str());
			}

			// needed to avoid elements texts from overlapping later on
			int y_text_offset = 0;

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
					(sequenceStartTime < timeInDisplayStart && sequenceEndTime > timeInDisplayEnd);

				if (sequenceInDisplay)
				{
					float width = childSize.x * (sequences[i]->getDuration() / mSequencePlayer->getDuration());
					float start_x = childSize.x * (sequences[i]->getStartTime() / mSequencePlayer->getDuration());

					// draw sequence line
					drawList->AddLine(
						ImVec2(topLeft.x + start_x, topLeft.y),
						ImVec2(topLeft.x + start_x, topLeft.y + childSize.y + 100),
						colorWhite);

					// draw sequence text
					drawList->AddText(
						ImVec2(topLeft.x + start_x + 5, topLeft.y + childSize.y + 55),
						colorWhite,
						sequences[i]->mName.c_str());

					// draw sequence box
					ImVec2 sequenceBoxUpperLeft = ImVec2(topLeft.x + start_x, topLeft.y + childSize.y + 75);
					ImVec2 sequenceBoxLowerRight = ImVec2(topLeft.x + start_x + 25, topLeft.y + childSize.y + 100);

					drawList->AddRect(
						sequenceBoxUpperLeft,
						sequenceBoxLowerRight,
						colorWhite);

					// rename sequence action
					if (currentTimelineAction == TimeLineActions::NONE)
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

								selectedSequence = sequences[i];
								currentTimelineAction = TimeLineActions::SEQUENCE_RENAME_POPUP;
								inPopup = true;
							}
							else
							{
								if (showToolTips)
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
							(elementStartTime < timeInDisplayStart && elementEndTime > timeInDisplayEnd);

						if (elementInDisplay)
						{
							float element_pos = (element->getStartTime() - sequences[i]->getStartTime()) / sequences[i]->getDuration();
							float element_width = element->mDuration / sequences[i]->getDuration();

							// the bottom line
							drawList->AddLine(
								ImVec2(topLeft.x + start_x + width * element_pos, bottomRightPos.y),
								ImVec2(topLeft.x + start_x + width * element_pos, bottomRightPos.y + 50),
								colorLightGrey);

							// line in timeline
							drawList->AddLine(
								ImVec2(topLeft.x + start_x + width * element_pos, topLeft.y),
								ImVec2(topLeft.x + start_x + width * element_pos, bottomRightPos.y + 50),
								colorLightGrey);

							// the text
							drawList->AddText(
								ImVec2(topLeft.x + start_x + width * element_pos + 5, bottomRightPos.y + y_text_offset),
								colorLightGrey,
								element->mName.c_str());

							// draw dragger of element, changes duration of previous element
							{
								ImVec2 elementTimeDragRectStart(topLeft.x + start_x + width * element_pos, bottomRightPos.y + 40);
								ImVec2 elementTimeDragRectEnd(topLeft.x + start_x + width * element_pos + 10, bottomRightPos.y + 50);

								bool filled = false;
								if (currentTimelineAction == TimeLineActions::NONE)
								{
									if (ImGui::IsMouseHoveringRect(
										elementTimeDragRectStart,
										elementTimeDragRectEnd, true))
									{
										filled = true;

										if (ImGui::IsMouseClicked(0))
										{
											dirty = true;
											currentTimelineAction = TimeLineActions::DRAGGING_ELEMENT;
											selectedElement = element;
										}
										else if (ImGui::IsMouseClicked(1))
										{
											// rename sequence action
											if (currentTimelineAction == TimeLineActions::NONE)
											{
												outPopupOpened = true;
												outPopupId = "ElementActions";

												selectedElement = element;
												currentTimelineAction = TimeLineActions::ELEMENT_RENAME_POPUP;
												inPopup = true;
											}
										}
										else
										{
											if (showToolTips)
											{
												ImGui::BeginTooltip();
												ImGui::Text("Left click and hold to drag position, right click to show element actions");
												ImGui::EndTooltip();
											}
										}
									}
								}

								if (currentTimelineAction == TimeLineActions::DRAGGING_ELEMENT
									&& ImGui::IsMouseDragging()
									&& element == selectedElement)
								{
									dirty = true;

									filled = true;

									ImVec2 mousePs = ImGui::GetMousePos();
									double time = ((mousePs.x - topLeft.x) / childSize.x) * mSequencePlayer->getDuration();

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
							y_text_offset += 20;
							if (y_text_offset > 50 - 10)
							{
								y_text_offset = 0;
							}

							bool drawHandlers = false;
							bool draggingMotor = false;
							// draw motor end positions in element
							{
								float motor_height = childSize.y / 8.0f;
								for (int m = 0; m < 8; m++)
								{
									if ((enableMotorHandlerIndexBitMask >> m) & 1UL)
									{
										const float circleSize = 6.0f;

										bool filled = false;
										float x = topLeft.x + start_x + width * element_pos + width * element_width;
										float value = static_cast<ParameterFloat*>(element->getEndParameters()[m])->mValue;
										float y = (bottomRightPos.y - motor_height * (float)m) - motor_height * value;

										// handle dragging of motor values
										if (currentTimelineAction == TimeLineActions::NONE)
										{
											if (ImGui::IsMouseHoveringRect(ImVec2(x - 12, y - 12), ImVec2(x + 12, y + 12)))
											{
												filled = true;
												if (ImGui::IsMouseClicked(0))
												{
													currentTimelineAction = TimeLineActions::DRAGGING_MOTORVALUE;
													motorDragged = m;
													selectedElement = element;
													dirty = true;
												}
											}
										}

										if (currentTimelineAction == TimeLineActions::DRAGGING_MOTORVALUE
											&& ImGui::IsMouseDragging() &&
											m == motorDragged &&
											element == selectedElement)
										{
											filled = true;
											dirty = true;

											ImVec2 mousePos = ImGui::GetMousePos();

											float adjust = (mousePos.y - y) / motor_height;
											float newValue = static_cast<ParameterFloat*>(element->getEndParameters()[m])->mValue;
											newValue -= adjust;
											newValue = math::clamp(newValue, 0.0f, 1.0f);
											static_cast<ParameterFloat*>(element->getEndParameters()[m])->mValue = newValue;

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
							float element_size_width = element_width * width;

							// only applies to transition elements
							timeline::SequenceTransition* transition = dynamic_cast<timeline::SequenceTransition*>(element);
							if (transition != nullptr)
							{
								// get curves of this transition
								const auto& curves = transition->getCurves();

								// motor height is size of motor timeline in child height
								float motor_height = childSize.y / 8.0f;
								for (int m = 0; m < 8; m++)
								{
									if ((enableMotorHandlerIndexBitMask >> m) & 1UL)
									{
										//
										dirty = true;

										// get the range of the difference between start and finish
										float range = static_cast<ParameterFloat*>(element->getEndParameters()[m])->mValue - static_cast<ParameterFloat*>(element->getStartParameters()[m])->mValue;

										// get the start position
										float start_curve = static_cast<ParameterFloat*>(element->getStartParameters()[m])->mValue;

										// is mouse hovering in this element part ?
										bool mouseInMotor = false;

										if (ImGui::IsMouseHoveringRect(
											ImVec2(topLeft.x + start_x + width * element_pos, bottomRightPos.y - motor_height * (m + 1)),
											ImVec2(topLeft.x + start_x + width * element_pos + element_size_width, bottomRightPos.y - motor_height * m)))
										{
											mouseInMotor = true;

											if (showToolTips)
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
											currentTimelineAction == TimeLineActions::NONE)
										{
											currentTimelineAction = TimeLineActions::ADD_CURVEPOINT;

											dirty = true;

											// range can be negative, if curves moves downwards, thats why we need to flip the input later on
											bool flip = range < 0.0f;

											// to make sure p_y doesnt become infinite
											if (math::abs(range) > 0.00001f)
											{
												ImVec2 mousePos = ImGui::GetMousePos();

												// translate mouse pos to curve pos
												float p_x = (mousePos.x - (topLeft.x + start_x + width * element_pos)) / element_size_width;
												float p_y = (((bottomRightPos.y - motor_height * (float)m) - mousePos.y) / motor_height) * range;

												if (flip)
												{
													p_y = 1.0f + p_y;
												}

												// create the new point
												auto newPoint = curves[m]->mPoints[0];
												newPoint.mPos.mTime = p_x;
												newPoint.mPos.mValue = math::clamp(p_y, 0.0f, 1.0f);

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
											float x = topLeft.x + start_x + width * element_pos + element_size_width * point.mPos.mTime;
											float y = (bottomRightPos.y - motor_height * (float)m) - motor_height * ((point.mPos.mValue * range) + start_curve);

											// draw the position
											drawList->AddCircleFilled(
												ImVec2(x, y),
												3,
												colorWhite,
												12);

											// are we hovering inside the control point ?
											if (ImGui::IsMouseHoveringRect(ImVec2(x - 5, y - 5), ImVec2(x + 5, y + 5)) &&
												currentTimelineAction == TimeLineActions::NONE)
											{
												if (ImGui::IsMouseClicked(0))
												{
													if (!ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_X)))
													{
														curvePointPtr = &point;
														currentTimelineAction = TimeLineActions::DRAGGING_CURVEPOINT;
													}
													else
													{
														// handle deletion

														// store the index so we can delete it later
														pointsToDelete.emplace_back(p);

														currentTimelineAction = TimeLineActions::DELETE_CURVEPOINT;
													}

													dirty = true;
												}
											}

											//
											ImVec2 pointPos(x, y);

											// handle dragging of curve point
											if (currentTimelineAction == TimeLineActions::DRAGGING_CURVEPOINT &&
												curvePointPtr == &point)
											{
												ImVec2 mousePos = ImGui::GetMousePos();

												// translate to curve space
												float adjust = (mousePos.y - y) / motor_height;

												point.mPos.mValue -= adjust * (range / 1.0f);
												point.mPos.mValue = math::clamp(point.mPos.mValue, 0.0f, 1.0f);

												adjust = (mousePos.x - x) / element_size_width;
												point.mPos.mTime += adjust;
												point.mPos.mTime = math::clamp(point.mPos.mTime, 0.0f, 1.0f);
											
												dirty = true;
											}

											// translate 
											x = topLeft.x + start_x + width * element_pos + element_size_width * (point.mPos.mTime + point.mOutTan.mTime);
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
												currentTimelineAction == TimeLineActions::NONE)
											{
												if (ImGui::IsMouseClicked(0))
												{
													currentTimelineAction = TimeLineActions::DRAGGING_TANGENT;
													tangentPtr = &point.mOutTan;

													dirty = true;
												}
											}

											// handle dragging of tangent
											if (currentTimelineAction == TimeLineActions::DRAGGING_TANGENT &&
												tangentPtr == &point.mOutTan)
											{
												ImVec2 mousePos = ImGui::GetMousePos();

												// translate
												float adjust = (mousePos.y - y) / motor_height;
												tangentPtr->mValue -= adjust * (range / 1.0f);

												adjust = (mousePos.x - x) / element_size_width;
												tangentPtr->mTime += adjust;

												dirty = true;
											}

											// translate
											x = topLeft.x + start_x + width * element_pos + element_size_width * (point.mPos.mTime + point.mInTan.mTime);
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
												currentTimelineAction == TimeLineActions::NONE)
											{
												if (ImGui::IsMouseClicked(0))
												{
													currentTimelineAction = TimeLineActions::DRAGGING_TANGENT;
													tangentPtr = &point.mInTan;

													dirty = true;
												}
											}

											// handle dragging
											if (currentTimelineAction == TimeLineActions::DRAGGING_TANGENT &&
												tangentPtr == &point.mInTan)
											{
												ImVec2 mousePos = ImGui::GetMousePos();

												float adjust = (mousePos.y - y) / motor_height;
												tangentPtr->mValue -= adjust * (range / 1.0f);

												adjust = (mousePos.x - x) / element_size_width;
												tangentPtr->mTime += adjust;

												dirty = true;
											}

											if (currentTimelineAction == TimeLineActions::DRAGGING_TANGENT &&
												tangentPtr == &point.mOutTan)
											{
												point.mInTan.mTime = -tangentPtr->mTime;
												point.mInTan.mValue = -tangentPtr->mValue;

												curves[m]->invalidate();
												mSequencePlayer->reconstruct();
											}

											if (currentTimelineAction == TimeLineActions::DRAGGING_TANGENT &&
												tangentPtr == &point.mInTan)
											{
												point.mOutTan.mTime = -tangentPtr->mTime;
												point.mOutTan.mValue = -tangentPtr->mValue;

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
					bool showMotorInputs = true;
					if (showMotorInputs)
					{
						if (dirty)
						{
							//
							cachedCurve.clear();

							cachedCurve = std::vector<std::vector<ImVec2>>(8);

							// create parameters that we evaluate
							std::vector<std::unique_ptr<ParameterFloat>> parametersPts;
							std::vector<Parameter*> parameters;
							for (int p = 0; p < 8; p++)
							{
								parametersPts.emplace_back(std::make_unique<ParameterFloat>());
								parameters.emplace_back(parametersPts.back().get());
							}

							// zoom in on the part that is shown in the window
							int steps = totalCurveResolution;
							float part = windowWidth / childWidth;
							float part_start = math::clamp<float>(scrollX - 40, 0, childWidth) / childWidth;

							// start evaluating and create curves of motor
							for (int p = 0; p < steps; p++)
							{
								//
								mSequencePlayer->evaluate(((mSequencePlayer->getDuration() * part) / (float)steps) * (float)p + (mSequencePlayer->getDuration() * part_start), parameters);

								//
								for (int l = 0; l < 8; l++)
								{
									float y_part = (childSize.y / 8.0f);
									float y_start = y_part * l;

									cachedCurve[l] .emplace_back(ImVec2(
										part_start * childWidth + topLeft.x + childSize.x * part * (p * (1.0f / (float)steps)),
										bottomRightPos.y - y_start - y_part * static_cast<ParameterFloat*>(parameters[l])->mValue));
								}
							}

							dirty = false;
						}
			
						if (cachedCurve.size() == 8)
						{
							for (int l = 0; l < 8; l++)
							{
								if (cachedCurve[l].size() > 0)
								{
									// draw the polylines 
									drawList->AddPolyline(
										&*cachedCurve[l].begin(),
										cachedCurve[l].size(),
										colorRed,
										false,
										1.5f,
										true);
								}
							}
						}

					}
				}
			}

			for (int i = 0; i < 8; i++)
			{
				float y_pos = (childSize.y / 8) * i + 4;
				// draw motor text
				drawList->AddText(
					ImVec2(topLeft.x - 25, topLeft.y + y_pos),
					colorWhite,
					std::to_string(8 - i).c_str());

				//
				const ImVec2 rectTopLeft(topLeft.x - 15, topLeft.y + y_pos + 5);
				const ImVec2 rectBotRight(topLeft.x - 5, topLeft.y + y_pos + 15);

				bool filled = false;
				int motorId = 7 - i;
				if (currentTimelineAction == TimeLineActions::NONE)
				{
					if (ImGui::IsMouseHoveringRect(rectTopLeft, rectBotRight))
					{
						if (showToolTips)
						{
							ImGui::BeginTooltip();
							ImGui::Text("Left click to toggle curve edit mode");
							ImGui::EndTooltip();
						}

						filled = true;

						if (ImGui::IsMouseClicked(0))
						{
							if ( (enableMotorHandlerIndexBitMask >> motorId ) & 1UL )
							{
								enableMotorHandlerIndexBitMask &= ~(1UL << motorId);
							}
							else
							{
								enableMotorHandlerIndexBitMask |= 1UL << motorId;
							}
							currentTimelineAction = TimeLineActions::ENABLING_HANDLERS;
						}
					}
				}

				if (!filled)
					filled = (enableMotorHandlerIndexBitMask >> motorId) & 1UL;

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
			drawList->AddLine(ImVec2(topLeft.x + player_pos, topLeft.y), ImVec2(topLeft.x + player_pos, bottomRightPos.y),
				colorWhite);

			// time in seconds
			drawList->AddText(
				ImVec2(topLeft.x + player_pos + 5, topLeft.y - 20),
				colorRed,
				formatTimeString(mSequencePlayer->getCurrentTime()).c_str());

			// handle dragging of timeline
			if (ImGui::IsMouseHoveringRect(topLeft, bottomRightPos))
			{
				drawCursor = true;
				if (!inPopup)
				{
					ImVec2 mousePos = ImGui::GetMousePos();
					mousePos = ImVec2(mousePos.x - topLeft.x, mousePos.y - topLeft.y);
					float pos = mousePos.x / childSize.x;
					currentTimeOfMouseInSequence = pos * mSequencePlayer->getDuration();

					// set cursorpos 
					cursorPos = (currentTimeOfMouseInSequence / mSequencePlayer->getDuration()) * childSize.x;

					if (currentTimelineAction == TimeLineActions::NONE)
					{
						// is clicked inside timeline ? jump to position
						if (ImGui::IsMouseClicked(0))
						{
							currentTimelineAction = TimeLineActions::DRAGGING_PLAYERPOSITION;

							mSequencePlayer->setTime(currentTimeOfMouseInSequence);
						}
					}

					if (currentTimelineAction == TimeLineActions::DRAGGING_PLAYERPOSITION)
					{
						// handle drag in timeline
						if (ImGui::IsMouseDragging())
						{
							// translate mouseposition to position in sequenceplayer
							ImVec2 mousePos = ImGui::GetMousePos();
							mousePos = ImVec2(mousePos.x - topLeft.x, mousePos.y - topLeft.y);
							float pos = mousePos.x / childSize.x;
							currentTimeOfMouseInSequence = pos * mSequencePlayer->getDuration();
							mSequencePlayer->setTime(currentTimeOfMouseInSequence);
						}
					}
					else
					{
						if (showToolTips && !inPopup)
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
				if (!inPopup)
				{
					drawCursor = false;
				}
			}

			if ( drawCursor )
			{
				drawList->AddLine(
					ImVec2(topLeft.x + cursorPos, topLeft.y),
					ImVec2(topLeft.x + cursorPos, bottomRightPos.y),
					colorRed, 1.5f);
			}

			// handle insertion of elements or sequences
			if (currentTimelineAction == TimeLineActions::NONE &&
				ImGui::IsMouseClicked(1) &&
				ImGui::IsMouseHoveringRect(topLeft, bottomRightPos))
			{
				inPopup = true;
				currentTimelineAction = TimeLineActions::INSERTION_POPUP;

				outPopupOpened = true;
				outPopupId = "Insert";
			}
			ImGui::EndChild();

			//printf("%llu\n", draw_list->_VtxCurrentIdx);
			if (drawList->_VtxCurrentIdx > 64000)
			{
				printf("Draw list to large! Decrease resolution\n");
				drawList->Clear();
			}

			ImGui::Spacing();
			ImGui::Spacing();
		}
	}


	void FlexblockGui::drawTimelinePlayerControls(bool& outPopupOpened, std::string& popupId)
	{
		static auto showTip = [](const char* tip)
		{
			if (showToolTips)
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
		float cursorPosX = ImGui::GetCursorPosX();
		ImGui::SetCursorPos(ImVec2(cursorPosX + ImGui::GetScrollX(), ImGui::GetCursorPosY()));

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
		ImGui::Checkbox("Follow", &followPlayer);
		if (followPlayer)
		{
			dirty = true;
			ImGui::SetScrollX((mSequencePlayer->getCurrentTime() / mSequencePlayer->getDuration()) * childWidth);
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
		if (ImGui::SliderFloat("Vertical Zoom", &child_height, 350.0f, 1500.0f, ""))
			dirty = true;

		ImGui::PopItemWidth();

		showTip("Vertical zoom");

		// zoom of timeline
		ImGui::SameLine();
		ImGui::PushItemWidth(100.0f);
		if (ImGui::SliderFloat("Horizontal Zoom", &lengthPerSecond, 4.0f, 60.0f, ""))
			dirty = true;
		ImGui::PopItemWidth();

		childWidth = (mSequencePlayer->getDuration() / lengthPerSecond) * ImGui::GetWindowWidth();
		
		showTip("Horizontal zoom");

		// curves resolution
		ImGui::SameLine();
		ImGui::PushItemWidth(100.0f);
		if (ImGui::SliderInt("Curve Res.", &totalCurveResolution, 25, 200, ""))
			dirty = true;
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

			inPopup = true;
			outPopupOpened = true;
			currentTimelineAction = TimeLineActions::SAVE_POPUP;
		}

		showTip("Save as new show");

		// handle load
		ImGui::SameLine();
		if (ImGui::Button("Load"))
		{
			popupId = "Load";
			inPopup = true;
			outPopupOpened = true;
			currentTimelineAction = TimeLineActions::LOAD_POPUP;
			dirty = true;
		}

		showTip("Load show");

		//
		ImGui::SameLine();
		ImGui::Text(("CurrentShow : " + mSequencePlayer->getShowName()).c_str());

		//
		ImGui::SameLine();
		ImGui::Checkbox("Show tooltips", &showToolTips);

		showTip("Toggle tooltips");

		ImGui::SameLine();
		if (ImGui::Checkbox("Show PlayList", &mShowPlaylist))
		{
			if (mShowPlaylist)
			{
				ImGui::SetNextTreeNodeOpen(true);
			}
		}

		showTip("Show list of sequences");
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

			ImGui::Combo("Shows", &selectedShowIndex, [](void* data, int index, const char** out_text)
			{
				ParameterService::PresetFileList* show_files = (ParameterService::PresetFileList*)data;
				*out_text = (*show_files)[index].data();
				return true;
			}, &shows, shows.size());

			utility::ErrorState errorState;
			if (ImGui::Button("Load"))
			{
				if (mSequencePlayer->load(files_in_directory[selectedShowIndex], errorState))
				{
					ImGui::CloseCurrentPopup();
					inPopup = false;
					currentTimelineAction = TimeLineActions::NONE;
				}
				else
				{
					errorString = errorState.toString();
					ImGui::OpenPopup("Failed to load show");
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
				inPopup = false;
				currentTimelineAction = TimeLineActions::NONE;
			}

			if (ImGui::BeginPopupModal("Failed to load show", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text(errorString.c_str());
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

			strcpy(*&buffer, selectedSequence->mName.c_str());

			if (ImGui::InputText("Rename", *&buffer, 256))
			{
				std::string newName(buffer);
				selectedSequence->mName = newName;
			}

			if (mSequencePlayer->getSequences().size() > 1)
			{
				if (ImGui::Button("Delete"))
				{
					mSequencePlayer->removeSequence(selectedSequence);
					dirty = true;

					inPopup = false;
					currentTimelineAction = TimeLineActions::NONE;
					ImGui::CloseCurrentPopup();
				}
			}

			if (ImGui::Button("Done"))
			{
				inPopup = false;
				currentTimelineAction = TimeLineActions::NONE;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		else
		{
			if (currentTimelineAction == TimeLineActions::SEQUENCE_RENAME_POPUP)
			{
				currentTimelineAction = TimeLineActions::NONE;
				inPopup = false;
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

			if (ImGui::Combo("Shows", &selectedShowIndex, [](void* data, int index, const char** out_text)
			{
				ParameterService::PresetFileList* show_files = (ParameterService::PresetFileList*)data;
				*out_text = (*show_files)[index].data();

				return true;
			}, &shows, shows.size()))
			{
				if (selectedShowIndex == shows.size() - 1)
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
					selectedShowIndex = shows.size() - 1;

					if (mSequencePlayer->load(newFilename, errorState))
					{

					}
					else
					{
						errorString = errorState.toString();
						ImGui::OpenPopup("Failed to load show");
					}
				}
				else
				{
					errorString = errorState.toString();
					ImGui::OpenPopup("Failed to save show");
				}
			}

			if (ImGui::BeginPopupModal("Overwrite"))
			{
				ImGui::Text(("Are you sure you want to overwrite " + shows[selectedShowIndex] + " ?").c_str());
				if (ImGui::Button("OK"))
				{
					if (mSequencePlayer->save(shows[selectedShowIndex], errorState))
					{
						if (mSequencePlayer->load(shows[selectedShowIndex], errorState))
						{

						}
						else
						{
							errorString = errorState.toString();
							ImGui::OpenPopup("Failed  to save show");
						}
					}
					else
					{
						errorString = errorState.toString();
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
				ImGui::Text(errorString.c_str());
				if (ImGui::Button("OK"))
				{
					ImGui::CloseCurrentPopup();
				}
				inPopup = false;
				currentTimelineAction = TimeLineActions::NONE;
				ImGui::EndPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Done"))
			{
				inPopup = false;
				currentTimelineAction = TimeLineActions::NONE;
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

			strcpy(*&buffer, selectedElement->mName.c_str());

			if (ImGui::InputText("Rename", *&buffer, 256))
			{
				std::string newName(buffer);
				selectedElement->mName = newName;
			}

			const timeline::Sequence* owningSequence = mSequencePlayer->getSequenceAtTime(selectedElement->getStartTime());

			if (ImGui::InputFloat("Duration", &selectedElement->mDuration, 0.1f, 0.2f, 2) )
			{
				if (selectedElement->mDuration < 0.01f)
				{
					selectedElement->mDuration = 0.01f;
				}

				mSequencePlayer->reconstruct();
				dirty = true;
			}

			// enable delete if sequence has more then one element
			if (owningSequence != nullptr &&
				owningSequence->getElements().size() > 1)
			{
				if (ImGui::Button("Delete"))
				{
					mSequencePlayer->removeSequenceElement(owningSequence, selectedElement);
					dirty = true;

					inPopup = false;
					currentTimelineAction = TimeLineActions::NONE;
					ImGui::CloseCurrentPopup();
				}
			}

			if (ImGui::Button("Done"))
			{
				inPopup = false;
				currentTimelineAction = TimeLineActions::NONE;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		else
		{
			if (currentTimelineAction == TimeLineActions::ELEMENT_RENAME_POPUP)
			{
				currentTimelineAction = TimeLineActions::NONE;
				inPopup = false;
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
			std::string errorString = "";

			// insert pause element at current time
			if (ImGui::Button("Insert Pause"))
			{
				utility::ErrorState errorState;
				bool success = insertNewElement(std::make_unique<timeline::SequencePause>(), errorState);

				if (!success)
				{
					// handle error
					ImGui::BeginPopup("Insert Error");
					errorString = errorState.toString();
				}
				else 
				{
					// exit popup
					inPopup = false;
					currentTimelineAction = TimeLineActions::NONE;
					ImGui::CloseCurrentPopup();
				}
			}

			// insert transition element
			if (ImGui::Button("Insert Transition"))
			{
				utility::ErrorState errorState;

				bool success = insertNewElement(std::make_unique<flexblock::FlexblockSequenceTransition>(), errorState);

				if (!success)
				{
					// handle error
					ImGui::BeginPopup("Insert Error");
					errorString = errorState.toString();
				}
				else
				{
					// exit popup
					inPopup = false;
					currentTimelineAction = TimeLineActions::NONE;
					ImGui::CloseCurrentPopup();
				}
			}

			// insert new sequence
			if (ImGui::Button("Insert Sequence"))
			{
				utility::ErrorState errorState;

				if (!insertNewSequence(std::make_unique<flexblock::FlexblockSequence>(), errorState))
				{
					ImGui::OpenPopup("Insert Error");
					errorString = errorState.toString();
				}
				else
				{
					inPopup = false;
					currentTimelineAction = TimeLineActions::NONE;
					ImGui::CloseCurrentPopup();
				}
			}

			if (ImGui::Button("Cancel"))
			{
				inPopup = false;
				currentTimelineAction = TimeLineActions::NONE;
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::BeginPopupModal("Insert Error"))
			{
				ImGui::Text(errorString.c_str());

				if (ImGui::Button("Ok"))
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			ImGui::EndPopup();
		}
		else
		{
			if (currentTimelineAction == TimeLineActions::INSERTION_POPUP)
			{
				currentTimelineAction = TimeLineActions::NONE;
				inPopup = false;
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


	bool FlexblockGui::insertNewElement(std::unique_ptr<timeline::SequenceElement> newElement, utility::ErrorState errorState)
	{
		dirty = true;

		double time = currentTimeOfMouseInSequence;
		mSequencePlayer->setTime(time);
		time = mSequencePlayer->getCurrentTime();

		// retrieve sequence and element at current time
		timeline::Sequence* sequence = mSequencePlayer->getSequenceAtTime(time);
		timeline::SequenceElement* element = sequence->getElementAtTime(time);

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


	bool FlexblockGui::insertNewSequence(std::unique_ptr<timeline::Sequence> newSequence, utility::ErrorState errorState)
	{
		dirty = true;

		// retrieve current sequence and element
		double time = currentTimeOfMouseInSequence;
		timeline::Sequence* sequence = mSequencePlayer->getSequenceAtTime(time);
		timeline::SequenceElement* element = sequence->getElementAtTime(time);

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

				// insert the new sequence
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
}