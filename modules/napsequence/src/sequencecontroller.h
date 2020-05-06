#pragma once

#include "sequenceplayer.h"

#include <nap/core.h>

namespace nap
{
	// forward declares
	class SequenceController;

	using SequenceControllerFactoryFunc = std::unique_ptr<SequenceController>(*)(SequencePlayer&);

	class NAPAPI SequenceController
	{
	public:
		SequenceController(SequencePlayer& player) : mPlayer(player) {};

		Sequence& getSequence() { return mPlayer.getSequence(); }

		/**
		 * save
		 * saves the sequence
		 */
		void save();

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

		virtual void insertTrack(rttr::type type) {}

		/**
		 * generic insert segment method
		 * type of track will be deduced from track id and a new segment of the right type will be inserted
		 * @param trackID the trackID in which to insert new segment
		 * @param time the time at which to insert the segment
		 */
		virtual void insertSegment(const std::string& trackID, double time) = 0;

		virtual void deleteSegment(const std::string& trackID, const std::string& segmentID) = 0;

		/**
		 * getSegment
		 * @param trackID the trackID
		 * @param segmentID the segmentID
		 * @return const pointer to tracksegment, returns nullptr when not found
		 */
		const SequenceTrackSegment* getSegment(const std::string& trackID, const std::string& segmentID) const;

		static std::unordered_map<rttr::type, SequenceControllerFactoryFunc>& getControllerFactory();

		static bool registerControllerFactory(rttr::type, SequenceControllerFactoryFunc);
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

		SequencePlayer& mPlayer;
	};
}
