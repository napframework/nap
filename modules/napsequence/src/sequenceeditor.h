#pragma once

// internal includes
#include "sequence.h"
#include "sequenceplayer.h"
#include "sequenceutils.h"
#include "sequenceeditortypes.h"
#include "sequencecontroller.h"

// external includes
#include <nap/resource.h>
#include <parameter.h>
#include <nap/logger.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceEditorController;
	class SequenceController;

	/**
	 * SequenceEditor
	 * The SequenceEditor is responsible for editing the sequence (model) and makes sure the model stays valid during editing.
	 * It also holds a resource ptr to a player, to make sure that editing the sequence stays thread safe.
	 */
	class NAPAPI SequenceEditor : 
		public Resource
	{
		friend class SequenceEditorGUI;

		RTTI_ENABLE(Resource)
	public:
		/**
		 * init
		 * initializes editor
		 * @param errorState contains any errors
		 * @return returns true on successful initialization
		*/
		virtual bool init(utility::ErrorState& errorState);

		template<typename T>
		T& getController() 
		{
			assert(mControllers.find(RTTI_OF(T)) != mControllers.end()); // type not found
			return static_cast<T&>(*mControllers[RTTI_OF(T)].get());
		}
	public:
		// properties
		ResourcePtr<SequencePlayer> mSequencePlayer = nullptr; ///< Property: 'Sequence Player' ResourcePtr to the sequence player

		/**
		 * getSequence
		 * @return const reference to sequence
		 */
		const Sequence& getSequence() const;
	protected:
		// the controller class
		std::unique_ptr<SequenceEditorController> mController = nullptr;

		std::unordered_map<rttr::type, std::unique_ptr<SequenceController>> mControllers;
	private:

		/**
		 * getController
		 * @return reference to controller
		 */
		SequenceEditorController& getController();
	};

	using UpdateSegmentsFunction = void(SequenceEditorController::*)(SequenceTrack&);

	/**
	 * SequenceEditorController 
	 * The actual controller with methods that a view can call
	 */
	class NAPAPI SequenceEditorController
	{
	public:
		/**
		 * Constructor
		 * @param sequencePlayer reference to the sequence player
		 * @param sequence reference to the sequence ( model )
		 */
		SequenceEditorController(SequencePlayer& sequencePlayer);
		
		/*
		/**
		 * segmentDurationChange
		 * @param trackID the id of the track
		 * @param segmentID the id of the segment we need to edit
		 * @param amount the amount that the duration of this segment should change
		 */
		void segmentDurationChange(const std::string& trackID, const std::string& segmentID, float amount);

		/**
		 * segmentEventStartTimeChange
		 * changes start time of segment
		 * @param trackID the trackID of track containing segment
		 * @param segmentID the segmentID
		 * @param amount the amount the starttime needs to change
		 */
		void segmentEventStartTimeChange(const std::string& trackID, const std::string& segmentID, float amount);

		/**
		 * save
		 * saves the sequence
		 */
		void save();

		/**
		 * deleteSegment
		 * delete segment 
		 * @param trackID the track in which the segment gets deleted 
		 * @param segmentID the segment ID that needs to be deleted
		 */
		void deleteSegment(const std::string& trackID, const std::string& segmentID);


		/**
		 * assignNewObjectID
		 * create an adapter for a specified object ( F.E. Parameters or Events ) for specified track
		 * @param trackID the track id that gets an assigned object
		 * @param objectID the object that is assigned to the track and used to create the adapter
		 */
		void assignNewObjectID(const std::string& trackID, const std::string& objectID);


		/**
		 * deleteTrack
		 * deletes a track
		 * @param deleteTrackID the id of the track that needs to be deleted
		 */
		void deleteTrack(const std::string& deleteTrackID);

		/**
		 * getSequencePlayer
		 * @return returns reference to sequence player
		 */
		SequencePlayer& getSequencePlayer() const;

		/**
		 * getSequence
		 * @return returns const reference to sequence of sequenceplayer
		 */
		const Sequence& getSequence() const;

		/**
		 * addCurveTrack
		 * adds a new curve track of type T ( float, vec2, vec3, vec4 )
		 */
		template<typename T>
		void addNewCurveTrack();

		/**
		 * adds a new event track
		 */
		void addNewEventTrack();

		/**
		 * generic insert segment method
		 * type of track will be deduced from track id and a new segment of the right type will be inserted
		 * @param trackID the trackID in which to insert new segment
		 * @param time the time at which to insert the segment
		 */
		void insertSegment(const std::string& trackID, double time);

		/**
		 * changeCurveSegmentValue
		 * changes start or end value of segment of type T
		 * @param trackID the track id
		 * @param segmentID id of segment
		 * @param amount the amount that the value needs to change
		 * @param curveIndex the curve index of the value 
		 * @param valueType the segment value type ( first or last value )
		 */
		void changeCurveSegmentValue(const std::string& trackID, const std::string& segmentID, float amount, int curveIndex, SequenceEditorTypes::SegmentValueTypes valueType);


		/**
		 * insertCurvePoint
		 * insert point in curve of segment
		 * @param trackID the track id
		 * @param segmentID the segment id
		 * @param pos the position at which to insert the curvepoint in curve ( range 0-1 )
		 * @param curveIndex the index of the curve 
		 */
		void insertCurvePoint(const std::string& trackID,const std::string& segmentID, float pos, int curveIndex);

		/**
		 * deleteCurvePoint
		 * deletes point from curve
		 * @param trackID the trackID
		 * @param segmentID the segmentID
		 * @param index the point index
		 * @param curveIndex the curveIndex
		 */
		void deleteCurvePoint(const std::string& trackID, const std::string& segmentID, const int index, int curveIndex);

		/**
		 * changeCurvePoint
		 * changes a curvepoint value and time / position
		 * @param trackID the trackID
		 * @param segmentID the segmentID
		 * @param pointIndex the point index
		 * @param curveIndex the curve index
		 * @param time offset for new time
		 * @param value offset for new value
		 */
		void changeCurvePoint(const std::string& trackID, const std::string& segmentID, const int pointIndex, const int curveIndex, float time,float value);

		/**
		 * changeTanPoint
		 * changes tangent of curve point. Tangents are always aligned
		 * @param trackID the trackID
		 * @param segmentID the segmentID
		 * @param pointIndex the point index
		 * @param curveIndex the curve index
		 * @param tanType in or out tangent
		 * @param time offset for new time
		 * @param value offset for new value
		 */
		void changeTanPoint(const std::string& trackID, const std::string& segmentID, const int pointIndex, const int curveIndex, SequenceEditorTypes::TanPointTypes tanType, float time, float value);

		/**
		 * changeMinMaxCurveTrack
		 * changes minimum and maximum value of track
		 * @param trackID the trackID
		 * @param minimum new minimum
		 * @param maximum new maximum
		 */
		template<typename T>
		void changeMinMaxCurveTrack(const std::string& trackID, T minimum, T maximum);

		/**
		 * changeCurveType
		 * changes curvetype ( linear or bezier )
		 * @param trackID the trackID
		 * @param segmentID the segmentID
		 * @param curveType the new curve type
		 */
		void changeCurveType(const std::string& trackID, const std::string& segmentID, math::ECurveInterp type);

		/**
		 * editEventSegment
		 * edits event message
		 * @param trackID the trackID
		 * @param segmentID the segmentID
		 * @param message the new message
		 */
		void editEventSegment(const std::string& trackID,const std::string& segmentID, const std::string& eventMessage);
		
		/**
		 * getSegment
		 * @param trackID the trackID
		 * @param segmentID the segmentID
		 * @return const pointer to tracksegment, returns nullptr when not found
		 */
		const SequenceTrackSegment* getSegment(const std::string& trackID, const std::string& segmentID) const;
	protected:

		/**
		 * findSegment
		 * finds segment
		 * @param trackID the trackID
		 * @param segmentID the segmentID
		 * @return raw pointer to tracksegment, returns nullptr when not found
		 */
		SequenceTrackSegment* findSegment(const std::string& trackID, const std::string& segmentID);
	
		/**
		 * findTrack
		 * finds segment
		 * @param trackID the trackID
		 * @return raw pointer to track, returns nullptr when not found
		 */
		SequenceTrack* findTrack(const std::string& trackID);

		/**
		 * deleteObjectFromSequencePlayer
		 * deletes an object owned by sequenceplayer from sequenceplayer
		 * @param id object id
		 */
		void deleteObjectFromSequencePlayer(const std::string& id);

		/**
		 * updateTrack
		 * updates duration of sequence by longest track
		 */
		void updateTracks();

		/**
		 * updateCurveSegments
		 * updates curve segments values to be continuous ( segment 1 end value == segment 2 start value etc )
		 * @param track reference to sequence track
		 */
		template<typename T>
		void updateCurveSegments(SequenceTrack& track);

		/**
		 * insertCurveSegment
		 * inserts a new curvesegment of type T ( vec2, vec3, vec4, float )
		 * @param trackID the track in which to insert the segment
		 * @param time the time at when to insert segment
		 */
		template <typename T>
		void insertCurveSegment(const std::string& trackID, double time);

		template <typename  T>
		void changeTanPoint(SequenceTrackSegment& segment, const std::string& trackID, const int pointIndex, const int curveIndex, SequenceEditorTypes::TanPointTypes tanType, float time, float value);

		template<typename T>
		void changeCurveType(SequenceTrackSegment& segment, math::ECurveInterp type);

		template <typename  T>
		void changeCurvePoint(SequenceTrackSegment& segment, const int pointIndex, const int curveIndex, float time, float value);

		template<typename T>
		void deleteCurvePoint(SequenceTrackSegment& segment, const int index, int curveIndex);

		template<typename T>
		void insertCurvePoint(SequenceTrackSegment& segment, float pos, int curveIndex);

		template<typename T>
		void changeCurveSegmentValue(SequenceTrack& track, SequenceTrackSegment& segment, float amount, int curveIndex, SequenceEditorTypes::SegmentValueTypes valueType);

		void insertEventSegment(const std::string& trackID, double time);
	protected:
		// reference to player
		SequencePlayer&		mSequencePlayer;
	private:
		static std::unordered_map<rttr::type, UpdateSegmentsFunction> sUpdateSegmentFunctionMap;
	};
}
