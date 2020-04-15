#pragma once

// internal includes
#include "sequence.h"
#include "sequenceplayer.h"
#include "sequenceutils.h"

// external includes
#include <nap/resource.h>
#include <parameter.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceEditorController;

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
		virtual bool init(utility::ErrorState& errorState);
	public:
		// properties
		ResourcePtr<SequencePlayer> mSequencePlayer = nullptr; ///< Property: 'Sequence Player' ResourcePtr to the sequence player

		const Sequence& getSequence() const;
	protected:
		std::unique_ptr<SequenceEditorController> mController = nullptr;
	private:
		SequenceEditorController& getController();
	};

	/**
	 * 
	 */
	namespace SequenceEditorTypes
	{
		enum TanPointTypes
		{
			IN,
			OUT
		};

		/**
		*
		*/
		enum SegmentValueTypes
		{
			BEGIN,
			END
		};
	}
	

	/**
	 * SequenceEditorController 
	 * The actual controller with methods that a view can call
	 */
	class SequenceEditorController
	{
	public:
		/**
		 * Constructor
		 * @param sequencePlayer reference to the sequence player
		 * @param sequence reference to the sequence ( model )
		 */
		SequenceEditorController(SequencePlayer& sequencePlayer) 
			: mSequencePlayer(sequencePlayer) {}

		/**
		 * segmentDurationChange
		 * @param segmentID the id of the segement we need to edit
		 * @param amount the amount that the duration of this segment should change
		 */
		void segmentDurationChange(
			const std::string& segmentID, 
			float amount);

		void segmentEventStartTimeChange(
			const std::string& trackID,
			const std::string& segmentID,
			float amount);

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
		void deleteSegment(
			const std::string& trackID,
			const std::string& segmentID);


		/**
		 * 
		 */
		void assignNewObjectID(
			const std::string& trackID,
			const std::string& parameterID);


		/**
		 * 
		 */
		void deleteTrack(const std::string& deleteTrackID);

		/**
		 * 
		 */
		SequencePlayer& getSequencePlayer() const;

		const Sequence& getSequence() const;

		//////////////////////////////////////////////////////////////////////////
		// Template Definitions
		//////////////////////////////////////////////////////////////////////////

		/**
		 * 
		 */
		template<typename T>
		void addNewCurveTrack();

		void addNewEventTrack();

		void insertEventSegment(
			const std::string& trackID,
			double time,
			const std::string& eventMessage );

		/**
		 * 
		 */
		template<typename T>
		void insertCurveSegment(
			const std::string& trackID,
			double time);

		/**
		 * 
		 */
		template<typename T>
		void changeCurveSegmentValue(
			const std::string& trackID,
			const std::string& segmentID,
			float amount,
			int curveIndex,
			SequenceEditorTypes::SegmentValueTypes valueType);

		/**
		 * 
		 */
		template<typename T>
		void updateSegments(SequenceTrack& track);

		/**
		 * 
		 */
		template<typename T>
		void insertCurvePoint(
			const std::string& trackID,
			const std::string& segmentID,
			float pos,
			int curveIndex);

		template<typename T>
		void deleteCurvePoint(
			const std::string& trackID,
			const std::string& segmentID,
			const int index,
			int curveIndex);

		template<typename T>
		void changeCurvePoint(
			const std::string& trackID,
			const std::string& segmentID,
			const int pointIndex,
			const int curveIndex,
			float time,
			float value);

		template<typename T>
		void changeTanPoint(
			const std::string& trackID,
			const std::string& segmentID,
			const int pointIndex,
			const int curveIndex,
			SequenceEditorTypes::TanPointTypes tanType,
			float time,
			float value);

		template<typename T>
		void changeMinMaxCurveTrack(const std::string& trackID, T minimum, T maximum);
	protected:

		SequenceTrackSegment* findSegment(const std::string& trackID, const std::string& segmentID);
	
		SequenceTrack* findTrack(const std::string& trackID);

		void deleteObjectFromSequencePlayer(const std::string& id);

		void updateTracks();
	protected:
		SequencePlayer&		mSequencePlayer;
	};
}
