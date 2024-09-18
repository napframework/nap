/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// internal includes
#include "sequence.h"
#include "sequencecontroller.h"
#include "sequencecurveenums.h"
#include "sequenceplayer.h"
#include "sequenceservice.h"
#include "sequenceservice.h"

// external includes
#include <nap/resource.h>
#include <parameter.h>
#include <nap/logger.h>
#include <atomic>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    // forward declares
    class SequenceController;

    /**
     * The SequenceEditor is responsible for editing the sequence (model) and makes sure the model stays valid during editing.
     * It also holds a resource ptr to a player, to make sure that editing the sequence stays thread safe.
     */
    class NAPAPI SequenceEditor : public Resource
    {
        friend class SequenceController;

    RTTI_ENABLE(Resource)
    public:
        SequenceEditor(SequenceService& service);

        /**
         * initializes editor
         * @param errorState contains any errors
         * @return returns true on successful initialization
        */
        bool init(utility::ErrorState& errorState) override;

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
         * Returns pointer to base class of controller type, asserts when type not found
         * @param trackType rttr::type information of controller type to be returned
         * @return ptr to controller base class, null ptr when not found
         */
        SequenceController *getControllerWithTrackType(rtti::TypeInfo trackType);

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
        T &getController()
        {
            assert(mControllers.find(RTTI_OF(T)) != mControllers.end()); // type not found
            return static_cast<T &>(*mControllers[RTTI_OF(T)].get());
        }


        /**
         * inserts marker at given time in seconds
         * @param time the time at where to insert the new marker in seconds
         * @param message const reference to the message that the new marker should contain
         */
        void insertMarker(double time, const std::string& message);

        /**
         * changes marker time, assert when markerID not found
         * @param markerID the id of the marker
         * @param time the new time in seconds for the marker
         */
        void changeMarkerTime(const std::string& markerID, double time);

        /**
         * deletes marker with specified id, assert when markerID not found
         * @param markerID the id of the marker to delete
         */
        void deleteMarker(const std::string& markerID);

        /**
         * changes marker message of specified marker id, asserts when marker not found
         * @param markerID the id of the marker to edit
         * @param markerMessage const reference to string value of marker message
         */
        void changeMarkerMessage(const std::string& markerID, const std::string& markerMessage);

        // properties
        ResourcePtr<SequencePlayer> mSequencePlayer = nullptr; ///< Property: 'Sequence Player' ResourcePtr to the sequence player
    private:
        // map of all controllers
        std::unordered_map<rttr::type, std::unique_ptr<SequenceController>> mControllers;

        /**
         * performs edit action when mutex of player is unlocked
         * @param action the edit action
         */
        void performEdit(std::function<void()> action);

        // make sure we don't perform two edit actions at the same time and make sure they are executed on the main thread
        // during the update call to the SequenceEditor
        std::atomic_bool mPerformingEditAction = {false};

        // service reference
        SequenceService& mService;
    };

    using SequenceEditorObjectCreator = rtti::ObjectCreator<SequenceEditor, SequenceService>;
}
