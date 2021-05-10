/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "sequenceplayer.h"
#include "sequence.h"

#include <nap/core.h>
#include <functional>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceController;
	class SequenceEditor;

	/**
	 * Base class for controllers for specific track types
	 */
	class NAPAPI SequenceController
	{
		RTTI_ENABLE()
	public:
		/**
		 * Constructor
		 * @param player reference to player being used
		 * @param editor reference to editor
		 */
		SequenceController(SequencePlayer& player, SequenceEditor& editor) : mPlayer(player), mEditor(editor) {};

		/**
		 * Deconstructor
		 */
		virtual ~SequenceController()= default;

		/**
		 * create an adapter for a specified object ( F.E. Parameters or Events ) for specified track
		 * @param trackID the track id that gets an assigned object
		 * @param objectID the object that is assigned to the track and used to create the adapter
		 */
		void assignNewObjectID(const std::string& trackID, const std::string& objectID);

		/**
		 * deletes a track
		 * @param deleteTrackID the id of the track that needs to be deleted
		 */
		void deleteTrack(const std::string& deleteTrackID);

		/**
		 * moves track up in the array of tracks
		 * @param trackID the id of the track that needs to be moved
		 */
		void moveTrackUp(const std::string& trackID);

		/**
		 * moves track up in the array of tracks
		 * @param trackID the id of the track that needs to be moved
		 */
		void moveTrackDown(const std::string& trackID);

		/**
		 * inserts track that corresponds to type of controller, must be overloaded
		 * @param type the type of track
		 */
		virtual void insertTrack(rttr::type type) = 0;

		/**
		 * inserts segment in track, must be overloaded
		 * @param trackID the trackID in which to insert new segment
		 * @param time the time at which to insert the segment
		 * @return const pointer to newly created segment
		 */
		virtual const SequenceTrackSegment* insertSegment(const std::string& trackID, double time) = 0;

		/**
		 * deleted segment from track, must be overloaded
		 * @param trackID the track
		 * @param segmentID the segment
		 */
		virtual void deleteSegment(const std::string& trackID, const std::string& segmentID) = 0;

		/**
		 *
		 * @param trackID
		 * @return
		 */
		const SequenceTrack* getTrack(const std::string& trackID) const;

		/**
		 * returns const segment pointer to specific segment, nullptr when not found
		 * @param trackID the trackID
		 * @param segmentID the segmentID
		 * @return const pointer to tracksegment, returns nullptr when not found
		 */
		const SequenceTrackSegment* getSegment(const std::string& trackID, const std::string& segmentID) const;
	protected:
		/**
		 * @return returns reference to sequence of player
		 */
		Sequence& getSequence() { return mPlayer.getSequence(); }

		/**
		 * finds segment
		 * @param trackID the trackID
		 * @param segmentID the segmentID
		 * @return raw pointer to tracksegment, returns nullptr when not found
		 */
		SequenceTrackSegment* findSegment(const std::string& trackID, const std::string& segmentID);

		/**
		 * finds segment
		 * @param trackID the trackID
	  	 * @return raw pointer to track, returns nullptr when not found
		 */
		SequenceTrack* findTrack(const std::string& trackID);

		/**
		 * deletes an object owned by sequenceplayer from sequenceplayer
		 * @param id object id
		 */
		void deleteObjectFromSequencePlayer(const std::string& id);

		/**
		 * updates duration of sequence by longest track
		 */
		void updateTracks();

		/**
		 * calls perform edit action on editor class
		 * @param action the edit action
		 */
		void performEditAction(std::function<void()> action);

		// objects owned by sequence player
		std::vector<std::unique_ptr<rtti::Object>>&	getPlayerOwnedObjects(){ return mPlayer.mReadObjects; };

		// read object ids from sequence
		std::unordered_set<std::string>& getPlayerReadObjectIDs(){ return mPlayer.mReadObjectIDs; };

		// reference to player
		SequencePlayer& mPlayer;

		// reference to editor
		SequenceEditor& mEditor;
	};
}