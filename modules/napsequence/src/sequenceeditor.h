/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// internal includes
#include "sequence.h"
#include "sequencecontroller.h"
#include "sequencecurveenums.h"
#include "sequenceplayer.h"
#include "sequenceutils.h"

// external includes
#include <nap/resource.h>
#include <parameter.h>
#include <nap/logger.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// forward declares
	class SequenceController;

	/**
	 * The SequenceEditor is responsible for editing the sequence (model) and makes sure the model stays valid during editing.
	 * It also holds a resource ptr to a player, to make sure that editing the sequence stays thread safe.
	 */
	class NAPAPI SequenceEditor : 
		public Resource
	{
		friend class SequenceController;

		RTTI_ENABLE(Resource)
	public:
		/**
		 * initializes editor
		 * @param errorState contains any errors
		 * @return returns true on successful initialization
		*/
		virtual bool init(utility::ErrorState& errorState);

		/**
		 * saves sequence of player to file
		 * @param file filename
		 */
		void save(const std::string& file);

		/**
		 * loads sequence of file
		 * @param file filename
		 */
		void load(const std::string& file);

		/**
		 * changes sequence duration
		 * @param newDuration new duration of sequence
		 */
		void changeSequenceDuration(double newDuration);

		/**
		 * Returns pointer to base class of controller type, returns nullptr when controller type is not found
		 * @param type rttr::type information of controller type to be returned
		 * @return ptr to controller base class, null ptr when not found
		 */
		SequenceController* getControllerWithTrackType(rttr::type type);

		/**
		 * Returns pointer to base class of controller type that is used for specified track type of track id
		 * Return null when track is not found, or controller is not found
		 * @param trackID the track id of the track to find controller for
		 * @return ptr to controller base class, null ptr when not found
		 */
		SequenceController* getControllerWithTrackID(const std::string& trackID);

		/**
		 * Gets reference the controller for a type, performs static cast
		 * @tparam T type of controller
		 * @return reference to controller type
		 */
		template<typename T>
		T& getController() 
		{
			assert(mControllers.find(RTTI_OF(T)) != mControllers.end()); // type not found
			return static_cast<T&>(*mControllers[RTTI_OF(T)].get());
		}

		/**
		 * Static method that registers a certain controller type for a certain view type, this can be used by views to map controller types to view types
		 * @param viewType the viewtype
		 * @param controllerType the controller type
		 * @return true on succesfull registration
		 */
		static bool registerControllerForTrackType(rttr::type viewType, rttr::type controllerType);

		void insertMarker(double time, const std::string& message);

		void changeMarkerTime(const std::string& markerID, double time);

		void deleteMarker(const std::string& markerID);

		void changeMarkerMessage(const std::string& markerID, const std::string& markerMessage);
	public:
		// properties
		ResourcePtr<SequencePlayer> mSequencePlayer = nullptr; ///< Property: 'Sequence Player' ResourcePtr to the sequence player
	private:
		// map of all controllers
		std::unordered_map<rttr::type, std::unique_ptr<SequenceController>> mControllers;

		/**
		 * performs edit action when mutex of player is unlocked, makes sure edit action are carried out thread safe, is blocking
		 * @param action the edit action
		 */
		void performEditAction(std::function<void()> action);

		// make sure we don't perform two edit actions at the same time ( only possible when performing an edit action inside another edit action )
		// edit actions are performed on player thread but block main thread
		bool mPerformingEditAction = false;
	};
}
