#pragma once

#include <sequenceeditorguiactions.h>
#include <sequencetracksegment.h>
#include <imgui/imgui.h>
#include <sequencetracksegmentevent.h>
#include <sequencecontrollerevent.h>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

	/**
	* Base class for an event segment view
	* This base class is used by the track view to draw and handle segment views of different event types
	* When you want to add your own event type, you can extend the derived class SequenceEventTrackSegmentView<T> where T is the value type of your own event
	*/
	class NAPAPI SequenceEventTrackSegmentViewBase
	{
		RTTI_ENABLE()
	public:
		/**
		* Constructor
		*/
		SequenceEventTrackSegmentViewBase() = default;

		/**
		* Deconstructor
		*/
		virtual ~SequenceEventTrackSegmentViewBase() = default;

		/**
		* Extend this method to implement the way editing this event type must be handle in the GUI
		* For examples, see template specializations in SequenceEventTrackView.cpp
		* @param action the incoming action from the gui, contains information about the track time and segment. Segment can be assumed to be of type SequenceTrackSegmentEvent<T>
		*/
		virtual void handleEditPopupContent(SequenceGUIActions::Action& action) = 0;

		/**
		* Extend this method to specify a way to draw this event type
		* For examples, see template specializations in SequenceEventTrackView.cpp
		* @param segment reference to segment
		* @param drawList pointer to ImGui drawlist
		* @param topLeft top left position
		* @param x x position of segment on track
		*/
		virtual void drawEvent(const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, float x) = 0;

		/**
		* Extend this method to specify the way the controller needs to be called to add your custom event type
		* Generally, this method doesn't need specialization
		* @param controller reference to controller
		* @param trackID id of event track
		* @param time time at which to insert custom event
		*/
		virtual void insertSegment(SequenceControllerEvent& controller, const std::string& trackID, double time) = 0;

		/**
		* Extend this method to specify the way an edit action for this event segment needs to be created
		* Generally, this method doesn't need specialization
		* @param segment const pointer to segment
		* @param trackID the track id
		* @param segmentID the segment id
		* @return unique pointer to created action, cannot be nullptr
		*/
		virtual std::unique_ptr<SequenceGUIActions::Action> createEditAction(const SequenceTrackSegmentEventBase* segment, const std::string& trackID, const std::string& segmentID) = 0;
	protected:
	};
}