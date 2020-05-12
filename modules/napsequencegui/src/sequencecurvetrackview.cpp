// local includes
#include "sequencecurvetrackview.h"
#include "sequenceeditorgui.h"
#include "sequencetrackcurve.h"
#include "napcolors.h"
#include "sequencecontrollercurve.h"
#include "sequenceplayercurveinput.h"

// nap includes
#include <nap/logger.h>
#include <parametervec.h>
#include <parameternumeric.h>

// external includes
#include <iostream>

namespace nap
{
	using namespace SequenceGUIActions;

	SequenceCurveTrackView::SequenceCurveTrackView(SequenceEditorGUIView& view, SequenceEditorGUIState& state)
		: SequenceTrackView(view, state)
	{
	}


	static bool registerCurveTrackView = SequenceTrackView::registerFactory(RTTI_OF(SequenceCurveTrackView), [](SequenceEditorGUIView& view, SequenceEditorGUIState& state)->std::unique_ptr<SequenceTrackView>
	{
		return std::make_unique<SequenceCurveTrackView>(view, state);
	});


	static bool curveViewRegistrations[4]
	{
		SequenceEditorGUIView::registerTrackViewType(RTTI_OF(SequenceTrackCurve<float>), RTTI_OF(SequenceCurveTrackView)),
		SequenceEditorGUIView::registerTrackViewType(RTTI_OF(SequenceTrackCurve<glm::vec2>), RTTI_OF(SequenceCurveTrackView)),
		SequenceEditorGUIView::registerTrackViewType(RTTI_OF(SequenceTrackCurve<glm::vec3>), RTTI_OF(SequenceCurveTrackView)),
		SequenceEditorGUIView::registerTrackViewType(RTTI_OF(SequenceTrackCurve<glm::vec4>), RTTI_OF(SequenceCurveTrackView))
	};


	std::unordered_map<rttr::type, SequenceCurveTrackView::DrawSegmentMemFunPtr> SequenceCurveTrackView::sDrawCurveSegmentsMap
	{
		{ RTTI_OF(SequenceTrackSegmentCurveFloat), &SequenceCurveTrackView::drawSegmentContent<float> },
		{ RTTI_OF(SequenceTrackSegmentCurveVec2), &SequenceCurveTrackView::drawSegmentContent<glm::vec2> },
		{ RTTI_OF(SequenceTrackSegmentCurveVec3), &SequenceCurveTrackView::drawSegmentContent<glm::vec3> },
		{ RTTI_OF(SequenceTrackSegmentCurveVec4), &SequenceCurveTrackView::drawSegmentContent<glm::vec4> }
	};


	static std::unordered_map<rttr::type, std::vector<rttr::type>> sParameterTypesForCurveType
		{
			{ RTTI_OF(SequenceTrackCurveFloat), { { RTTI_OF(ParameterFloat), RTTI_OF(ParameterDouble), RTTI_OF(ParameterLong), RTTI_OF(ParameterInt) } } },
			{ RTTI_OF(SequenceTrackCurveVec2), { { RTTI_OF(ParameterVec2) } } },
			{ RTTI_OF(SequenceTrackCurveVec3), { { RTTI_OF(ParameterVec3) } } }
		};


	static bool isParameterTypeAllowed(rttr::type curveType, rttr::type parameterType)
	{
		auto it = sParameterTypesForCurveType.find(curveType);
		if(it!=sParameterTypesForCurveType.end())
		{
			for(auto& type : sParameterTypesForCurveType[curveType])
			{
				if(parameterType == type)
					return true;
			}
		}

		return false;
	}


	void SequenceCurveTrackView::handlePopups()
	{
		handleInsertSegmentPopup();

		handleCurveTypePopup();

		handleInsertCurvePointPopup();

		handleDeleteSegmentPopup();

		handleCurvePointActionPopup();
	}


	void SequenceCurveTrackView::showInspectorContent(const SequenceTrack &track)
	{
		// draw the assigned parameter
		ImGui::Text("Assigned Parameter");

		ImVec2 inspectorCursorPos = ImGui::GetCursorPos();
		inspectorCursorPos.x += 5;
		inspectorCursorPos.y += 5;
		ImGui::SetCursorPos(inspectorCursorPos);

		bool assigned = false;
		std::string assignedID;
		std::vector<std::string> curveInputs;
		int currentItem = 0;
		curveInputs.emplace_back("none");
		int count = 0;
		const Parameter* assignedParameter = nullptr;

		for (const auto& input : getEditor().mSequencePlayer->mInputs)
		{
			if (input.get()->get_type() == RTTI_OF(SequencePlayerCurveInput))
			{
				auto& curveInput = static_cast<SequencePlayerCurveInput&>(*input.get());

				if(curveInput.mParameter != nullptr)
				{
					if(isParameterTypeAllowed(track.get_type(), curveInput.mParameter.get()->get_type()))
					{
						count++;

						if (input->mID == track.mAssignedInputID)
						{
							assigned = true;
							assignedID = input->mID;
							currentItem = count;

							assert(input.get()->get_type() == RTTI_OF(SequencePlayerCurveInput)); // type mismatch
							assignedParameter = static_cast<SequencePlayerCurveInput*>(input.get())->mParameter.get();
						}

						curveInputs.emplace_back(input->mID);
					}
				}
			}
		}

		ImGui::PushItemWidth(140.0f);
		if (Combo(
			"",
			&currentItem,
			curveInputs))
		{
			SequenceControllerCurve& curveController = getEditor().getController<SequenceControllerCurve>();

			if (currentItem != 0)
				curveController.assignNewObjectID(track.mID, curveInputs[currentItem]);
			else
				curveController.assignNewObjectID(track.mID, "");
		}

		//
		ImGui::PopItemWidth();

		// map of inspectors ranges for curve types
		static std::unordered_map<rttr::type, void(SequenceCurveTrackView::*)(const SequenceTrack&)> sInspectors
			{
				{ RTTI_OF(SequenceTrackCurveFloat) , &SequenceCurveTrackView::drawInspectorRange<float> },
				{ RTTI_OF(SequenceTrackCurveVec2) , &SequenceCurveTrackView::drawInspectorRange<glm::vec2> },
				{ RTTI_OF(SequenceTrackCurveVec3) , &SequenceCurveTrackView::drawInspectorRange<glm::vec3> },
				{ RTTI_OF(SequenceTrackCurveVec4) , &SequenceCurveTrackView::drawInspectorRange<glm::vec4> }
			};

		// draw inspector
		auto it = sInspectors.find(track.get_type());
		assert(it!=sInspectors.end()); // type not found
		if(it != sInspectors.end())
		{
			(*this.*it->second)(track);
		}

		// delete track button
		ImGui::Spacing();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
	}


	void SequenceCurveTrackView::showTrackContent(const SequenceTrack& track, const ImVec2& trackTopLeft)
	{
		// if dirty, redraw all curves
		if (mState.mDirty)
		{
			mCurveCache.clear();
		}

		ImDrawList* drawList = ImGui::GetWindowDrawList();

		if (mState.mIsWindowFocused)
		{
			// handle insertion of segment
			if (mState.mAction->isAction<None>())
			{
				if (ImGui::IsMouseHoveringRect(
					trackTopLeft, // top left position
					{ trackTopLeft.x + mState.mTimelineWidth, trackTopLeft.y + mState.mTrackHeight }))
				{
					// position of mouse in track
					drawList->AddLine(
						{ mState.mMousePos.x, trackTopLeft.y }, // top left
						{ mState.mMousePos.x, trackTopLeft.y + mState.mTrackHeight }, // bottom right
						guicolors::lightGrey, // color
						1.0f); // thickness

					ImGui::BeginTooltip();

					ImGui::Text(formatTimeString(mState.mMouseCursorTime).c_str());

					ImGui::EndTooltip();

					// right mouse down
					if (ImGui::IsMouseClicked(1))
					{
						double time = mState.mMouseCursorTime;

						//
						mState.mAction = createAction<OpenInsertSegmentPopup>(track.mID, time, track.get_type());
					}
				}
			}

			// draw line in track while in inserting segment popup
			if (mState.mAction->isAction<OpenInsertSegmentPopup>())
			{
				auto* action = mState.mAction->getDerived<OpenInsertSegmentPopup>();

				if (action->mTrackID == track.mID)
				{
					// position of insertion in track
					drawList->AddLine(
						{ trackTopLeft.x + (float)action->mTime * mState.mStepSize, trackTopLeft.y }, // top left
						{ trackTopLeft.x + (float)action->mTime * mState.mStepSize, trackTopLeft.y + mState.mTrackHeight }, // bottom right
						guicolors::lightGrey, // color
						1.0f); // thickness
				}
			}

			// draw line in track while in inserting segment popup
			if (mState.mAction->isAction<InsertingSegment>())
			{
				auto* action = mState.mAction->getDerived<InsertingSegment>();

				if (action->mTrackID == track.mID)
				{
					// position of insertion in track
					drawList->AddLine(
						{ trackTopLeft.x + (float)action->mTime * mState.mStepSize, trackTopLeft.y }, // top left
						{ trackTopLeft.x + (float)action->mTime * mState.mStepSize, trackTopLeft.y + mState.mTrackHeight }, // bottom right
						guicolors::lightGrey, // color
						1.0f); // thickness
				}
			}
		}

		float previousSegmentX = 0.0f;

		int segmentCount = 0;
		for (const auto& segment : track.mSegments)
		{
			float segmentX = (segment->mStartTime + segment->mDuration) * mState.mStepSize;
			float segmentWidth = segment->mDuration * mState.mStepSize;

			auto it = sDrawCurveSegmentsMap.find(segment.get()->get_type());
			if (it != sDrawCurveSegmentsMap.end())
			{
				(*this.*it->second)(track, *segment.get(), trackTopLeft, previousSegmentX, segmentWidth, segmentX, drawList, (segmentCount == 0));
			}

			// draw segment handlers
			drawSegmentHandler(
				track,
				*segment.get(),
				trackTopLeft,
				segmentX,
				segmentWidth,
				drawList);

			//
			previousSegmentX = segmentX;

			//
			segmentCount++;
		}
	}


	template<typename T>
	void SequenceCurveTrackView::drawControlPoints(
		const SequenceTrack& track,
		const SequenceTrackSegment& segmentBase,
		const ImVec2 &trackTopLeft,
		const float segmentX,
		const float segmentWidth,
		ImDrawList* drawList)
	{

		const SequenceTrackSegmentCurve<T>& segment
			= static_cast<const SequenceTrackSegmentCurve<T>&>(segmentBase);

		// draw first control point(s) handlers IF this is the first segment of the track
		if (track.mSegments[0]->mID == segment.mID)
		{
			for (int v = 0; v < segment.mCurves.size(); v++)
			{
				const auto& curvePoint = segment.mCurves[v]->mPoints[0];
				std::ostringstream stringStream;
				stringStream << segment.mID << "_point_" << 0 << "_curve_" << v;

				ImVec2 circlePoint =
				{ (trackTopLeft.x + segmentX - segmentWidth) + segmentWidth * curvePoint.mPos.mTime,
					trackTopLeft.y + mState.mTrackHeight * (1.0f - (float)curvePoint.mPos.mValue) };

				drawTanHandler<T>(
					track,
					segment,
					stringStream,
					segmentWidth,
					curvePoint,
					circlePoint,
					0,
					v,
					SequenceCurveEnums::TanPointTypes::IN,
					drawList);

				drawTanHandler<T>(
					track,
					segment,
					stringStream,
					segmentWidth,
					curvePoint,
					circlePoint,
					0,
					v,
					SequenceCurveEnums::TanPointTypes::OUT,
					drawList);
			}
		}

		// draw control points of curves
		// we ignore the first and last because they are controlled by the start & end value of the segment
		for (int v = 0; v < segment.mCurves.size(); v++)
		{
			for (int i = 1; i < segment.mCurves[v]->mPoints.size() - 1; i++)
			{
				// get the curvepoint and generate a unique ID for the control point
				const auto& curvePoint = segment.mCurves[v]->mPoints[i];
				std::ostringstream stringStream;
				stringStream << segment.mID << "_point_" << i << "_curve_" << v;
				std::string pointID = stringStream.str();

				// determine the point at where to draw the control point
				ImVec2 circlePoint =
				{ (trackTopLeft.x + segmentX - segmentWidth) + segmentWidth * curvePoint.mPos.mTime,
					trackTopLeft.y + mState.mTrackHeight * (1.0f - (float)curvePoint.mPos.mValue) };

				// handle mouse hovering
				bool hovered = false;
				if (mState.mIsWindowFocused)
				{
					if ((mState.mAction->isAction<None>() ||
						mState.mAction->isAction<HoveringControlPoint>() ||
						mState.mAction->isAction<HoveringCurve>())
						&& ImGui::IsMouseHoveringRect(
							{ circlePoint.x - 5, circlePoint.y - 5 },
							{ circlePoint.x + 5, circlePoint.y + 5 }))
					{
						hovered = true;
					}
				}

				if (hovered)
				{
					// if we are hovering this point, store ID
					mState.mAction = createAction<HoveringControlPoint>(
						track.mID,
						segment.mID,
						i,
						v);

					//
					showValue<T>(
						track,
						segment,
						curvePoint.mPos.mTime,
						curvePoint.mPos.mTime * segment.mDuration + segment.mStartTime,
						v);

					// is the mouse held down, then we are dragging
					if (ImGui::IsMouseDown(0))
					{
						mState.mAction = createAction<DraggingControlPoint>(
							track.mID,
							segment.mID,
							i,
							v);
					}
					// if we clicked right mouse button, open curve action popup
					else if (ImGui::IsMouseClicked(1))
					{
						mState.mAction = createAction<OpenCurvePointActionPopup>(
							track.mID,
							segment.mID,
							i,
							v);
					}
				}
				else
				{
					// otherwise, if we where hovering but not anymore, stop hovering
					if (mState.mAction->isAction<HoveringControlPoint>())
					{
						auto* action = mState.mAction->getDerived<HoveringControlPoint>();
						if (action->mControlPointIndex == i && track.mID == action->mTrackID && segment.mID == action->mSegmentID && v == action->mCurveIndex)
						{
							mState.mAction = createAction<None>();
						}
					}
				}

				if (mState.mIsWindowFocused)
				{
					// handle dragging of control point
					if (mState.mAction->isAction<DraggingControlPoint>())
					{
						auto* action = mState.mAction->getDerived<DraggingControlPoint>();

						if (action->mSegmentID == segment.mID)
						{
							if (action->mControlPointIndex == i && action->mCurveIndex == v)
							{
								float timeAdjust = mState.mMouseDelta.x / segmentWidth;
								float valueAdjust = (mState.mMouseDelta.y / mState.mTrackHeight) * -1.0f;

								hovered = true;

								showValue<T>(
									track,
									segment,
									curvePoint.mPos.mTime,
									curvePoint.mPos.mTime * segment.mDuration + segment.mStartTime,
									v);


								SequenceControllerCurve& curveController = getEditor().getController<SequenceControllerCurve>();

								curveController.changeCurvePoint(
									action->mTrackID,
									action->mSegmentID,
									action->mControlPointIndex,
									action->mCurveIndex,
									timeAdjust,
									valueAdjust);

								mCurveCache.clear();

								if (ImGui::IsMouseReleased(0))
								{
									mState.mAction = createAction<None>();
								}
							}
						}
					}
				}

				// draw the control point
				drawList->AddCircleFilled(
					circlePoint,
					4.0f,
					hovered ? guicolors::white : guicolors::lightGrey);

				// draw the handlers
				drawTanHandler<T>(
					track,
					segment,
					stringStream,
					segmentWidth,
					curvePoint,
					circlePoint,
					i,
					v,
					SequenceCurveEnums::TanPointTypes::IN,
					drawList);

				drawTanHandler<T>(
					track,
					segment,
					stringStream,
					segmentWidth,
					curvePoint,
					circlePoint,
					i,
					v,
					SequenceCurveEnums::TanPointTypes::OUT,
					drawList);
			}
		}

		for (int v = 0; v < segment.mCurves.size(); v++)
		{
			// handle last control point
			// overlaps with endvalue so only draw tan handlers
			const int controlPointIndex = segment.mCurves[v]->mPoints.size() - 1;
			const auto& curvePoint = segment.mCurves[v]->mPoints[controlPointIndex];

			std::ostringstream stringStream;
			stringStream << segment.mID << "_point_" << controlPointIndex << "_curve_" << v;
			std::string pointID = stringStream.str();

			ImVec2 circlePoint =
			{ (trackTopLeft.x + segmentX - segmentWidth) + segmentWidth * curvePoint.mPos.mTime,
				trackTopLeft.y + mState.mTrackHeight * (1.0f - (float)curvePoint.mPos.mValue) };

			drawTanHandler<T>(
				track,
				segment,
				stringStream,
				segmentWidth,
				curvePoint,
				circlePoint,
				controlPointIndex,
				v,
				SequenceCurveEnums::TanPointTypes::IN,
				drawList);

			drawTanHandler<T>(
				track,
				segment,
				stringStream,
				segmentWidth,
				curvePoint,
				circlePoint,
				controlPointIndex,
				v,
				SequenceCurveEnums::TanPointTypes::OUT,
				drawList);
		}

		//
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - mState.mTrackHeight);
	}

	template<typename T>
	void SequenceCurveTrackView::drawSegmentValue(
		const SequenceTrack& track,
		const SequenceTrackSegment& segmentBase,
		const ImVec2 &trackTopLeft,
		const float segmentX,
		const float segmentWidth,
		const SequenceCurveEnums::SegmentValueTypes segmentType,
		ImDrawList* drawList)
	{
		const SequenceTrackSegmentCurve<T>& segment =
			static_cast<const SequenceTrackSegmentCurve<T>&>(segmentBase);

		for (int v = 0; v < segment.mCurves.size(); v++)
		{
			// calculate point of this value in the window
			ImVec2 segmentValuePos =
			{
				trackTopLeft.x + segmentX - (segmentType == SequenceCurveEnums::BEGIN ? segmentWidth : 0.0f),
				trackTopLeft.y + mState.mTrackHeight * (1.0f - ((segmentType == SequenceCurveEnums::BEGIN ?
					(float)segment.mCurves[v]->mPoints[0].mPos.mValue :
					(float)segment.mCurves[v]->mPoints[segment.mCurves[v]->mPoints.size() - 1].mPos.mValue) / 1.0f))
			};

			bool hovered = false;

			if (mState.mIsWindowFocused)
			{
				// check if we are hovering this value
				if ((mState.mAction->isAction<None>() ||
					mState.mAction->isAction<HoveringSegmentValue>() ||
					mState.mAction->isAction<HoveringSegment>() ||
					mState.mAction->isAction<HoveringCurve>())
					&& ImGui::IsMouseHoveringRect(
						{ segmentValuePos.x - 12, segmentValuePos.y - 12 }, // top left
						{ segmentValuePos.x + 12, segmentValuePos.y + 12 }))  // bottom right 
				{
					hovered = true;
					mState.mAction = createAction<HoveringSegmentValue>(
						track.mID,
						segment.mID,
						segmentType,
						v);

					if (ImGui::IsMouseDown(0))
					{
						mState.mAction = createAction<DraggingSegmentValue>(
							track.mID,
							segment.mID,
							segmentType,
							v);
					}

					showValue<T>(
						track,
						segment,
						segmentType == SequenceCurveEnums::BEGIN ? 0.0f : 1.0f,
						segmentType == SequenceCurveEnums::BEGIN ? segment.mStartTime : segment.mStartTime + segment.mDuration,
						v);
				}
				else if (!mState.mAction->isAction<DraggingSegmentValue>())
				{
					if (mState.mAction->isAction<HoveringSegmentValue>())
					{
						auto* action = mState.mAction->getDerived<HoveringSegmentValue>();

						if (action->mType == segmentType &&
							action->mSegmentID == segment.mID &&
							action->mCurveIndex == v)
						{
							mState.mAction = createAction<None>();
						}
					}
				}

				// handle dragging segment value
				if (mState.mAction->isAction<DraggingSegmentValue>())
				{
					auto* action = mState.mAction->getDerived<DraggingSegmentValue>();
					if (action->mSegmentID == segment.mID)
					{
						if (action->mType == segmentType && action->mCurveIndex == v)
						{
							hovered = true;
							showValue<T>(
								track,
								segment,
								segmentType == SequenceCurveEnums::BEGIN ? 0.0f : 1.0f,
								segmentType == SequenceCurveEnums::BEGIN ? segment.mStartTime : segment.mStartTime + segment.mDuration,
								v);

							if (ImGui::IsMouseReleased(0))
							{
								mState.mAction = createAction<None>();
							}
							else
							{
								float dragAmount = (mState.mMouseDelta.y / mState.mTrackHeight) * -1.0f;

								SequenceControllerCurve& curveController = getEditor().getController<SequenceControllerCurve>();

								curveController.changeCurveSegmentValue(
									track.mID,
									segment.mID,
									dragAmount,
									v,
									segmentType);
								mCurveCache.clear();
							}
						}
					}
				}
			}

			if (hovered)
				drawList->AddCircleFilled(segmentValuePos, 5.0f, guicolors::curvecolors[v]);
			else
				drawList->AddCircle(segmentValuePos, 5.0f, guicolors::curvecolors[v]);
		}
	}

	void SequenceCurveTrackView::drawSegmentHandler(
		const SequenceTrack& track,
		const SequenceTrackSegment& segment,
		const ImVec2 &trackTopLeft,
		const float segmentX,
		const float segmentWidth,
		ImDrawList* drawList)
	{
		// segment handler
		if (mState.mIsWindowFocused &&
			(!mState.mAction->isAction<DraggingSegment>() && !mState.mAction->isAction<DraggingSegmentValue>() && !mState.mAction->isAction<HoveringSegmentValue>())
			&& ImGui::IsMouseHoveringRect(
					{ trackTopLeft.x + segmentX - 10, trackTopLeft.y - 10 }, // top left
					{ trackTopLeft.x + segmentX + 10, trackTopLeft.y + mState.mTrackHeight + 10 }))  // bottom right 
		{
			
			// draw handler of segment duration
			drawList->AddLine(
			{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
			{ trackTopLeft.x + segmentX, trackTopLeft.y + mState.mTrackHeight }, // bottom right
				guicolors::white, // color
				3.0f); // thickness

			// we are hovering this segment with the mouse
			mState.mAction = createAction<HoveringSegment>(track.mID, segment.mID);

			ImGui::BeginTooltip();
			ImGui::Text(formatTimeString(segment.mStartTime).c_str());
			ImGui::EndTooltip();

			// left mouse is start dragging
			if (ImGui::IsMouseDown(0))
			{
				mState.mAction = createAction<DraggingSegment>(track.mID, segment.mID);
			}
			// right mouse in deletion popup
			else if (ImGui::IsMouseDown(1))
			{
				mState.mAction = createAction <OpenEditCurveSegmentPopup>(
					track.mID,
					segment.mID,
					segment.get_type()
				);
			}
		}
		else if (mState.mAction->isAction<DraggingSegment>())
		{
			auto* action = mState.mAction->getDerived<DraggingSegment>();
			if (action->mSegmentID == segment.mID)
			{
				// draw handler of segment duration
				drawList->AddLine(
				{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
				{ trackTopLeft.x + segmentX, trackTopLeft.y + mState.mTrackHeight }, // bottom right
					guicolors::white, // color
					3.0f); // thickness

				ImGui::BeginTooltip();
				ImGui::Text(formatTimeString(segment.mStartTime).c_str());
				ImGui::EndTooltip();

				// do we have the mouse still held down ? drag the segment
				if (ImGui::IsMouseDown(0))
				{
					float amount = mState.mMouseDelta.x / mState.mStepSize;

					auto& editor = getEditor();
					SequenceControllerCurve& curveController = editor.getController<SequenceControllerCurve>();
					curveController.segmentDurationChange(track.mID, segment.mID, amount);

					mCurveCache.clear();
				}
				// otherwise... release!
				else if (ImGui::IsMouseReleased(0))
				{
					mState.mAction = createAction<None>();
				}
			}
			else
			{
				// draw handler of segment duration
				drawList->AddLine(
				{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
				{ trackTopLeft.x + segmentX, trackTopLeft.y + mState.mTrackHeight }, // bottom right
					guicolors::white, // color
					1.0f); // thickness
			}
		}
		else
		{
			// draw handler of segment duration
			drawList->AddLine(
			{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
			{ trackTopLeft.x + segmentX, trackTopLeft.y + mState.mTrackHeight }, // bottom right
				guicolors::white, // color
				1.0f); // thickness

			// release if we are not hovering this segment
			if (mState.mAction->isAction<HoveringSegment>()
				&& mState.mAction->getDerived<HoveringSegment>()->mSegmentID == segment.mID)
			{
				mState.mAction = createAction<None>();
			}
		}
	}

	template<typename T>
	void SequenceCurveTrackView::drawTanHandler(
		const SequenceTrack &track,
		const SequenceTrackSegment &segment,
		std::ostringstream &stringStream,
		const float segmentWidth,
		const math::FCurvePoint<float, float> &curvePoint,
		const ImVec2 &circlePoint,
		const int controlPointIndex,
		const int curveIndex,
		const SequenceCurveEnums::TanPointTypes type,
		ImDrawList* drawList)
	{
		// draw tan handlers
		{
			// create a string stream to create identifier of this object
			std::ostringstream tanStream;
			tanStream << stringStream.str() << (type == SequenceCurveEnums::TanPointTypes::IN) ? "inTan" : "outTan";

			//
			const math::FComplex<float, float>& tanComplex
				= (type == SequenceCurveEnums::TanPointTypes::IN) ? curvePoint.mInTan : curvePoint.mOutTan;

			// get the offset from the tan
			ImVec2 offset =
			{ (segmentWidth * tanComplex.mTime) / (float)segment.mDuration,
				(mState.mTrackHeight *  (float)tanComplex.mValue * -1.0f) };
			ImVec2 tanPoint = { circlePoint.x + offset.x, circlePoint.y + offset.y };

			// set if we are hoverting this point with the mouse
			bool tanPointHovered = false;

			if (mState.mIsWindowFocused)
			{
				// check if hovered
				if ((mState.mAction->isAction<None>() || mState.mAction->isAction<HoveringCurve>())
					&& ImGui::IsMouseHoveringRect({ tanPoint.x - 5, tanPoint.y - 5 }, { tanPoint.x + 5, tanPoint.y + 5 }))
				{
					mState.mAction = createAction<HoveringTanPoint>(tanStream.str());
					tanPointHovered = true;
				}
				else if (
					mState.mAction->isAction<HoveringTanPoint>())
				{
					auto* action = mState.mAction->getDerived<HoveringTanPoint>();

					// if we hare already hovering, check if its this point
					if (action->mTanPointID == tanStream.str())
					{
						if (ImGui::IsMouseHoveringRect(
							{ tanPoint.x - 5, tanPoint.y - 5 },
							{ tanPoint.x + 5, tanPoint.y + 5 }))
						{
							// still hovering
							tanPointHovered = true;

							// start dragging if mouse down
							if (ImGui::IsMouseDown(0))
							{
								mState.mAction = createAction<DraggingTanPoint>(
									track.mID,
									segment.mID,
									controlPointIndex,
									curveIndex,
									type);
							}
						}
						else
						{
							// otherwise, release!
							mState.mAction = createAction<None>();
						}
					}
				}

				// handle dragging of tan point
				if (mState.mAction->isAction<DraggingTanPoint>())
				{
					auto* action = mState.mAction->getDerived<DraggingTanPoint>();

					if (action->mSegmentID == segment.mID &&
						action->mControlPointIndex == controlPointIndex &&
						action->mType == type &&
						action->mCurveIndex == curveIndex)
					{
						if (ImGui::IsMouseReleased(0))
						{
							mState.mAction = createAction<None>();
						}
						else
						{
							tanPointHovered = true;

							float time = mState.mMouseDelta.x / mState.mStepSize;
							float value = (mState.mMouseDelta.y / mState.mTrackHeight) * -1.0f;

							auto& curveController = getEditor().getController<SequenceControllerCurve>();
							curveController.changeTanPoint(
								track.mID,
								segment.mID,
								controlPointIndex,
								curveIndex,
								type,
								time,
								value);
							mCurveCache.clear();
						}
					}
				}
			}

			// draw line
			drawList->AddLine(circlePoint, tanPoint, tanPointHovered ? guicolors::white : guicolors::darkGrey, 1.0f);

			// draw handler
			drawList->AddCircleFilled(tanPoint, 3.0f, tanPointHovered ? guicolors::white : guicolors::darkGrey);
		}
	}


	void SequenceCurveTrackView::handleInsertSegmentPopup()
	{
		if (mState.mAction->isAction<OpenInsertSegmentPopup>())
		{
			auto* action = mState.mAction->getDerived<OpenInsertSegmentPopup>();

			if (action->mTrackType == RTTI_OF(SequenceTrackCurveFloat) ||
				action->mTrackType == RTTI_OF(SequenceTrackCurveVec2) ||
				action->mTrackType == RTTI_OF(SequenceTrackCurveVec3) ||
				action->mTrackType == RTTI_OF(SequenceTrackCurveVec4))
			{
				// invoke insert sequence popup
				ImGui::OpenPopup("Insert Segment");

				auto* action = mState.mAction->getDerived<OpenInsertSegmentPopup>();

				mState.mAction = createAction<InsertingSegment>(action->mTrackID, action->mTime, action->mTrackType);
			}
		}

		// handle insert segment popup
		if (mState.mAction->isAction<InsertingSegment>())
		{
			auto* action = mState.mAction->getDerived<InsertingSegment>();

			if (action->mTrackType == RTTI_OF(SequenceTrackCurveFloat) ||
				action->mTrackType == RTTI_OF(SequenceTrackCurveVec2) ||
				action->mTrackType == RTTI_OF(SequenceTrackCurveVec3) ||
				action->mTrackType == RTTI_OF(SequenceTrackCurveVec4))
			{
				if (ImGui::BeginPopup("Insert Segment"))
				{
					if (ImGui::Button("Insert"))
					{
						auto* action = mState.mAction->getDerived<InsertingSegment>();

						auto& curveController = getEditor().getController<SequenceControllerCurve>();
						curveController.insertSegment(action->mTrackID, action->mTime);
						mState.mAction = createAction<None>();

						mCurveCache.clear();

						ImGui::CloseCurrentPopup();

					}

					if (ImGui::Button("Cancel"))
					{
						ImGui::CloseCurrentPopup();
						mState.mAction = createAction<None>();
					}

					ImGui::EndPopup();
				}
				else
				{
					// click outside popup so cancel action
					mState.mAction = createAction<None>();
				}
			}
		}
	}


	void SequenceCurveTrackView::handleCurveTypePopup()
	{
		if (mState.mAction->isAction<OpenCurveTypePopup>())
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Change Curve Type");

			auto* action = mState.mAction->getDerived<OpenCurveTypePopup>();
			mState.mAction = createAction<CurveTypePopup>(
				action->mTrackID, 
				action->mSegmentID,
				action->mCurveIndex, 
				action->mPos,
				action->mWindowPos);
		}

		// handle insert segment popup
		if (mState.mAction->isAction<CurveTypePopup>())
		{
			auto* action = mState.mAction->getDerived<CurveTypePopup>();

			if (ImGui::BeginPopup("Change Curve Type"))
			{
				ImGui::SetWindowPos(action->mWindowPos);

				if (ImGui::Button("Linear"))
				{
					auto& curveController = getEditor().getController<SequenceControllerCurve>();
					curveController.changeCurveType(action->mTrackID, action->mSegmentID, math::ECurveInterp::Linear);

					ImGui::CloseCurrentPopup();
					mState.mAction = createAction<None>();
					mCurveCache.clear();

				}

				if (ImGui::Button("Bezier"))
				{
					auto& curveController = getEditor().getController<SequenceControllerCurve>();
					curveController.changeCurveType(action->mTrackID, action->mSegmentID, math::ECurveInterp::Bezier);

					ImGui::CloseCurrentPopup();
					mState.mAction = createAction<None>();
					mCurveCache.clear();
				}

				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
					mState.mAction = createAction<None>();
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState.mAction = createAction<None>();
			}
		}
	}


	void SequenceCurveTrackView::handleInsertCurvePointPopup()
	{
		if (mState.mAction->isAction<OpenInsertCurvePointPopup>())
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Insert Curve Point");

			auto* action = mState.mAction->getDerived<OpenInsertCurvePointPopup>();
			mState.mAction = createAction<InsertingCurvePoint>(action->mTrackID, action->mSegmentID, action->mSelectedIndex, action->mPos);
		}

		// handle insert segment popup
		if (mState.mAction->isAction<InsertingCurvePoint>())
		{
			if (ImGui::BeginPopup("Insert Curve Point"))
			{
				auto* action = mState.mAction->getDerived<InsertingCurvePoint>();
				if (ImGui::Button("Insert Point"))
				{
					auto& curveController = getEditor().getController<SequenceControllerCurve>();
					curveController.insertCurvePoint(
						action->mTrackID,
						action->mSegmentID,
						action->mPos,
						action->mSelectedIndex);

					mCurveCache.clear();

					ImGui::CloseCurrentPopup();
					mState.mAction = createAction<None>();

				}

				if (ImGui::Button("Change Curve Type"))
				{
					ImGui::CloseCurrentPopup();

					mState.mAction = createAction<OpenCurveTypePopup>(
						action->mTrackID,
						action->mSegmentID,
						action->mSelectedIndex,
						action->mPos,
						ImGui::GetWindowPos());
				}

				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
					mState.mAction = createAction<None>();
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState.mAction = createAction<None>();
			}
		}
	}


	void SequenceCurveTrackView::handleCurvePointActionPopup()
	{
		if (mState.mAction->isAction<OpenCurvePointActionPopup>())
		{
			auto* action = mState.mAction->getDerived<OpenCurvePointActionPopup>();
			mState.mAction = createAction<CurvePointActionPopup>(
					action->mTrackID,
					action->mSegmentID,
					action->mControlPointIndex,
					action->mCurveIndex
				);
			ImGui::OpenPopup("Curve Point Actions");
		}

		if (mState.mAction->isAction<CurvePointActionPopup>())
		{
			if (ImGui::BeginPopup("Curve Point Actions"))
			{
				if (ImGui::Button("Delete"))
				{
					auto* action = mState.mAction->getDerived<CurvePointActionPopup>();

					auto& curveController = getEditor().getController<SequenceControllerCurve>();
					curveController.deleteCurvePoint(
						action->mTrackID,
						action->mSegmentID,
						action->mControlPointIndex,
						action->mCurveIndex);
					mCurveCache.clear();

					mState.mAction = createAction<None>();

					ImGui::CloseCurrentPopup();
				}

				if (ImGui::Button("Cancel"))
				{
					mState.mAction = createAction<None>();

					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState.mAction = createAction<None>();
			}
		}
	}


	void SequenceCurveTrackView::handleDeleteSegmentPopup()
	{
		if (mState.mAction->isAction<OpenEditCurveSegmentPopup>())
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Delete Segment");

			auto* action = mState.mAction->getDerived<OpenEditCurveSegmentPopup>();

			mState.mAction = createAction<EditingCurveSegment>(
				action->mTrackID,
				action->mSegmentID,
				action->mSegmentType
			);
		}

		// handle delete segment popup
		if (mState.mAction->isAction<EditingCurveSegment>())
		{
			if (ImGui::BeginPopup("Delete Segment"))
			{
				auto* action = mState.mAction->getDerived<EditingCurveSegment>();

				if (ImGui::Button("Delete"))
				{
					getEditor().getController<SequenceControllerCurve>().deleteSegment(
						action->mTrackID,
						action->mSegmentID);
					mCurveCache.clear();

					ImGui::CloseCurrentPopup();
					mState.mAction = createAction<None>();
				}

				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
					mState.mAction = createAction<None>();
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState.mAction = createAction<None>();
			}
		}
	}


	template<typename T>
	void SequenceCurveTrackView::drawCurves(
		const SequenceTrack& track,
		const SequenceTrackSegment& segmentBase,
		const ImVec2 &trackTopLeft,
		const float previousSegmentX,
		const float segmentWidth,
		const float segmentX,
		ImDrawList* drawList)
	{
		const SequenceTrackSegmentCurve<T>& segment
			= static_cast<const SequenceTrackSegmentCurve<T>&>(segmentBase);

		const float pointsPerPixel = 0.5f;
		bool curveSelected = false;

		bool needsDrawing = ImGui::IsRectVisible({ trackTopLeft.x + previousSegmentX, trackTopLeft.y }, { trackTopLeft.x + previousSegmentX + segmentWidth, trackTopLeft.y + mState.mTrackHeight });

		if (needsDrawing)
		{
			// if no cache present, create new curve
			if (mCurveCache.find(segment.mID) == mCurveCache.end())
			{
				const int pointNum = (int) ( pointsPerPixel * segmentWidth );
				std::vector<std::vector<ImVec2>> curves;
				for (int v = 0; v < segment.mCurves.size(); v++)
				{
					std::vector<ImVec2> curve;
					for (int i = 0; i <= pointNum; i++)
					{
						float value = 1.0f - segment.mCurves[v]->evaluate((float)i / pointNum);

						ImVec2 point =
							{
								trackTopLeft.x + previousSegmentX + segmentWidth * ((float)i / pointNum),
								trackTopLeft.y + value * mState.mTrackHeight
							};

						if( ImGui::IsRectVisible(point, { point.x + 1, point.y + 1 }) )
						{
							curve.emplace_back(point);
						}
					}
					curves.emplace_back(curve);
				}
				mCurveCache.emplace(segment.mID, curves);
			}
		}

		int selectedCurve = -1;
		if (mState.mIsWindowFocused)
		{
			// determine if mouse is hovering curve
			if ((mState.mAction->isAction<None>() || mState.mAction->isAction<HoveringCurve>())
				&& ImGui::IsMouseHoveringRect(
					{ trackTopLeft.x + segmentX - segmentWidth, trackTopLeft.y }, // top left
					{ trackTopLeft.x + segmentX, trackTopLeft.y + mState.mTrackHeight }))  // bottom right 
			{
				// translate mouse position to position in curve
				ImVec2 mousePos = ImGui::GetMousePos();
				float xInSegment = ((mousePos.x - (trackTopLeft.x + segmentX - segmentWidth)) / mState.mStepSize) / segment.mDuration;
				float yInSegment = 1.0f - ((mousePos.y - trackTopLeft.y) / mState.mTrackHeight);

				for (int i = 0; i < segment.mCurves.size(); i++)
				{
					// evaluate curve at x position
					float yInCurve = segment.mCurves[i]->evaluate(xInSegment);

					// insert curve point on click
					const float maxDist = 0.1f;
					if (abs(yInCurve - yInSegment) < maxDist)
					{
						mState.mAction = createAction<HoveringCurve>(
							track.mID,
							segment.mID,
							i);

						if (ImGui::IsMouseClicked(1))
						{
							mState.mAction = createAction<OpenInsertCurvePointPopup>(
								track.mID,
								segment.mID,
								i,
								xInSegment);
						}
						selectedCurve = i;
					}
				}

				if (selectedCurve == -1)
				{
					mState.mAction = createAction<None>();
				}
				else
				{
					showValue<T>(
						track,
						segment,
						xInSegment,
						mState.mMouseCursorTime,
						selectedCurve);
				}
			}
			else
			{
				if (mState.mAction->isAction<HoveringCurve>())
				{
					auto* action = mState.mAction->getDerived<HoveringCurve>();

					if (action->mSegmentID == segment.mID)
					{
						mState.mAction = createAction<None>();
					}
				}
			}
		}

		if (needsDrawing)
		{
			for (int i = 0; i < segment.mCurves.size(); i++)
			{
				if (mCurveCache[segment.mID][i].size() > 0)
				{
					// draw points of curve
					drawList->AddPolyline(
						&*mCurveCache[segment.mID][i].begin(), // points array
						mCurveCache[segment.mID][i].size(),	 // size of points array
						guicolors::curvecolors[i],  // color
						false, // closed
						selectedCurve == i ? 3.0f : 1.0f, // thickness
						true); // anti-aliased
				}
			}
		}
	}

	template<typename T>
	void SequenceCurveTrackView::drawSegmentContent(
		const SequenceTrack &track,
		const SequenceTrackSegment &segment,
		const ImVec2& trackTopLeft,
		float previousSegmentX,
		float segmentWidth,
		float segmentX,
		ImDrawList* drawList,
		bool drawStartValue)
	{
		// curve
		drawCurves<T>(
			track,
			segment,
			trackTopLeft,
			previousSegmentX,
			segmentWidth,
			segmentX,
			drawList);


		// draw control points
		drawControlPoints<T>(
			track,
			segment,
			trackTopLeft,
			segmentX,
			segmentWidth,
			drawList);

		// if this is the first segment of the track
		// also draw a handler for the start value
		if (drawStartValue)
		{
			// draw segment value handler
			drawSegmentValue<T>(
				track,
				segment,
				trackTopLeft,
				segmentX,
				segmentWidth,
								SequenceCurveEnums::SegmentValueTypes::BEGIN,
				drawList);
		}

		// draw segment value handler
		drawSegmentValue<T>(
			track,
			segment,
			trackTopLeft,
			segmentX,
			segmentWidth,
							SequenceCurveEnums::SegmentValueTypes::END,
			drawList);
	}


	template<typename T>
	void SequenceCurveTrackView::drawInspectorRange(const SequenceTrack& track)
	{
		const SequenceTrackCurve<T>& curveTrack = static_cast<const SequenceTrackCurve<T>&>(track);

		T min = curveTrack.mMinimum;
		T max = curveTrack.mMaximum;

		//
		ImGui::PushID(track.mID.c_str());

		float dragFloatX = ImGui::GetCursorPosX() + 40;
		ImGui::SetCursorPos({ ImGui::GetCursorPosX() + 5, ImGui::GetCursorPosY() + 5 });
		ImGui::Text("Min:"); ImGui::SameLine();
		ImGui::PushID("min");
		ImGui::SetCursorPosX(dragFloatX);
		if (inputFloat<T>(min, 3))
		{
			auto& curveController = getEditor().getController<SequenceControllerCurve>();
			curveController.changeMinMaxCurveTrack<T>(track.mID, min, max);
		}
		ImGui::PopID();
		ImGui::PopItemWidth();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
		ImGui::Text("Max:"); ImGui::SameLine();
		ImGui::PushID("max");
		ImGui::SetCursorPosX(dragFloatX);
		if (inputFloat<T>(max, 3))
		{
			auto& curveController = getEditor().getController<SequenceControllerCurve>();
			curveController.changeMinMaxCurveTrack<T>(track.mID, min, max);
		}
		ImGui::PopID();
		ImGui::PopItemWidth();

		ImGui::PopID();
	}

	template<typename T>
	void SequenceCurveTrackView::showValue(
		const SequenceTrack& track,
		const SequenceTrackSegmentCurve<T>& segment,
		float x,
		double time,
		int curveIndex)
	{
		assert(false);
	}

	template<>
	void SequenceCurveTrackView::showValue<float>(
		const SequenceTrack& track,
		const SequenceTrackSegmentCurve<float>& segment,
		float x,
		double time,
		int curveIndex)
	{
		const SequenceTrackCurve<float>& curveTrack = static_cast<const SequenceTrackCurve<float>&>(track);

		ImGui::BeginTooltip();

		ImGui::Text(formatTimeString(time).c_str());
		ImGui::Text("%.3f", segment.getValue(x) * (curveTrack.mMaximum - curveTrack.mMinimum) + curveTrack.mMinimum);

		ImGui::EndTooltip();
	}

	template<>
	void SequenceCurveTrackView::showValue<glm::vec2>(
		const SequenceTrack& track,
		const SequenceTrackSegmentCurve<glm::vec2>& segment,
		float x,
		double time,
		int curveIndex)
	{
		assert(curveIndex >= 0);
		assert(curveIndex < 2);

		const SequenceTrackCurve<glm::vec2>& curveTrack = static_cast<const SequenceTrackCurve<glm::vec2>&>(track);

		ImGui::BeginTooltip();

		glm::vec2 value = segment.getValue(x) * (curveTrack.mMaximum - curveTrack.mMinimum) + curveTrack.mMinimum;

		static std::string names[2] =
		{
			"x",
			"y"
		};

		ImGui::Text(formatTimeString(time).c_str());
		ImGui::Text("%s : %.3f", names[curveIndex].c_str(), value[curveIndex]);

		ImGui::EndTooltip();
	}

	template<>
	void SequenceCurveTrackView::showValue<glm::vec3>(
		const SequenceTrack& track,
		const SequenceTrackSegmentCurve<glm::vec3>& segment,
		float x,
		double time,
		int curveIndex)
	{
		assert(curveIndex >= 0);
		assert(curveIndex < 3);

		const SequenceTrackCurve<glm::vec3>& curveTrack = static_cast<const SequenceTrackCurve<glm::vec3>&>(track);

		ImGui::BeginTooltip();

		glm::vec3 value = segment.getValue(x) * (curveTrack.mMaximum - curveTrack.mMinimum) + curveTrack.mMinimum;

		static std::string names[3] =
		{
			"x",
			"y",
			"z"
		};

		ImGui::Text(formatTimeString(time).c_str());
		ImGui::Text("%s : %.3f", names[curveIndex].c_str(), value[curveIndex]);

		ImGui::EndTooltip();
	}

	template<>
	void SequenceCurveTrackView::showValue<glm::vec4>(
		const SequenceTrack& track,
		const SequenceTrackSegmentCurve<glm::vec4>& segment,
		float x,
		double time,
		int curveIndex)
	{
		assert(curveIndex >= 0);
		assert(curveIndex < 4);

		const SequenceTrackCurve<glm::vec4>& curveTrack = static_cast<const SequenceTrackCurve<glm::vec4>&>(track);

		ImGui::BeginTooltip();

		glm::vec4 value = segment.getValue(x) * (curveTrack.mMaximum - curveTrack.mMinimum) + curveTrack.mMinimum;

		static std::string names[4] =
		{
			"x",
			"y",
			"z",
			"w"
		};

		ImGui::Text(formatTimeString(time).c_str());
		ImGui::Text("%s : %.3f", names[curveIndex].c_str(), value[curveIndex]);

		ImGui::EndTooltip();
	}

	template<typename T>
	bool SequenceCurveTrackView::inputFloat(T &v, int precision)
	{
		assert(true);
		return false;
	}

	template<>
	bool SequenceCurveTrackView::inputFloat<float>(float &v, int precision)
	{
		ImGui::PushItemWidth(100.0f);
		return ImGui::InputFloat("", &v, 0.0f, 0.0f, precision);
	}

	template<>
	bool SequenceCurveTrackView::inputFloat<glm::vec2>(glm::vec2 &v, int precision)
	{
		ImGui::PushItemWidth(145.0f);
		return ImGui::InputFloat2("", &v[0], precision);
	}

	template<>
	bool SequenceCurveTrackView::inputFloat<glm::vec3>(glm::vec3 &v, int precision)
	{
		ImGui::PushItemWidth(180.0f);
		return ImGui::InputFloat3("", &v[0], precision);
	}

	template<>
	bool SequenceCurveTrackView::inputFloat<glm::vec4>(glm::vec4 &v, int precision)
	{
		ImGui::PushItemWidth(225.0f);
		return ImGui::InputFloat4("", &v[0], precision);
	}
}
