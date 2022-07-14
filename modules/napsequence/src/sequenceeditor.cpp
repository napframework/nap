/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "sequenceeditor.h"

// external includes
#include <fcurve.h>
#include <functional>
#include <mathutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceEditor)
        RTTI_PROPERTY("Sequence Player", &nap::SequenceEditor::mSequencePlayer, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

using namespace nap::sequencecurveenums;

namespace nap
{
    SequenceEditor::SequenceEditor(SequenceService &service)
        : mService(service)
    {
    }


    bool SequenceEditor::init(utility::ErrorState &errorState)
    {
        if(!Resource::init(errorState))
        {
            return false;
        }

        // create controllers for all types of tracks
        const auto &controller_types = mService.getRegisteredControllerTypes();
        for(const auto &controller_type: controller_types)
        {
            mControllers.emplace(controller_type, mService.invokeControllerFactory(controller_type, *mSequencePlayer.get(), *this));
        }

        return true;
    }


    SequenceController *SequenceEditor::getControllerWithTrackID(const std::string &trackID)
    {
        const auto &sequence = mSequencePlayer->getSequence();
        for(const auto &track: sequence.mTracks)
        {
            if(trackID == track->mID)
            {
                auto track_ptr = track.get();
                auto track_type = track_ptr->get_type();
                auto controller_type = mService.getControllerTypeForTrackType(track_type);
                assert(mControllers.find(controller_type) != mControllers.end()); // entry not found
                return mControllers[controller_type].get();
            }
        }
        return nullptr;
    }


    void SequenceEditor::save(const std::string &file)
    {
        utility::ErrorState error_state;
        if(!mSequencePlayer->save(file, error_state))
        {
            nap::Logger::error(error_state.toString());
        }
    }


    void SequenceEditor::load(const std::string &file)
    {
        performEdit([this, file]()
                    {
                        utility::ErrorState error_state;
                        if(!mSequencePlayer->load(file, error_state))
                        {
                            nap::Logger::error(error_state.toString());
                        }
                    });
    }


    void SequenceEditor::changeSequenceDuration(double newDuration)
    {
        newDuration = math::max<double>(newDuration, 0.01);

        performEdit([this, newDuration]()
                    {
                        auto &sequence = mSequencePlayer->getSequence();

                        // sequence must be at least as long as longest track
                        double longest_track = 0.0;
                        for(auto &track: sequence.mTracks)
                        {
                            double longest_segment = 0.0;
                            for(auto &segment: track->mSegments)
                            {
                                double time = segment->mStartTime + segment->mDuration;
                                longest_segment = math::max<double>(longest_segment, time);
                            }
                            longest_track = math::max<double>(longest_segment, longest_track);
                        }

                        sequence.mDuration = math::max<double>(longest_track, newDuration);
                    });

        // reset player position if its bigger then duration
        if(mSequencePlayer->getPlayerTime() > newDuration)
        {
            mSequencePlayer->setPlayerTime(newDuration);
        }
    }


    void SequenceEditor::insertMarker(double time, const std::string &message)
    {
        performEdit([this, time, message]()
                    {
                        auto new_marker = std::make_unique<SequenceMarker>();
                        new_marker->mID = mService.generateUniqueID(mSequencePlayer->mReadObjectIDs);
                        new_marker->mTime = time;
                        new_marker->mMessage = message;

                        mSequencePlayer->mSequence->mMarkers.emplace_back(ResourcePtr<SequenceMarker>(new_marker.get()));
                        mSequencePlayer->mReadObjects.emplace_back(std::move(new_marker));
                    });
    }


    void SequenceEditor::changeMarkerTime(const std::string &markerID, double time)
    {
        // clamp time
        if(time < 0.0)
            time = 0.0;
        
        performEdit([this, markerID, time]()
                    {
                        auto it = std::find_if(mSequencePlayer->mSequence->mMarkers.begin(), mSequencePlayer->mSequence->mMarkers.end(), [markerID](
                            ResourcePtr<SequenceMarker> &a) -> bool
                        {
                            return markerID == a->mID;
                        });

                        assert(it != mSequencePlayer->mSequence->mMarkers.end());

                        if(it != mSequencePlayer->mSequence->mMarkers.end())
                        {
                            it->get()->mTime = time;
                        }
                    });

    }


    void SequenceEditor::deleteMarker(const std::string &markerID)
    {
        performEdit([this, markerID]()
                    {
                        auto it_1 = std::find_if(mSequencePlayer->mSequence->mMarkers.begin(), mSequencePlayer->mSequence->mMarkers.end(), [markerID](
                            ResourcePtr<SequenceMarker> &a) -> bool
                        {
                            return markerID == a->mID;
                        });
                        assert(it_1 != mSequencePlayer->mSequence->mMarkers.end());

                        if(it_1 != mSequencePlayer->mSequence->mMarkers.end())
                        {
                            mSequencePlayer->mSequence->mMarkers.erase(it_1);
                        }

                        auto it_2 = std::find_if(mSequencePlayer->mReadObjects.begin(), mSequencePlayer->mReadObjects.end(), [markerID](
                            std::unique_ptr<rtti::Object> &a) -> bool
                        {
                            return markerID == a->mID;
                        });
                        assert(it_2 != mSequencePlayer->mReadObjects.end());

                        if(it_2 != mSequencePlayer->mReadObjects.end())
                        {
                            mSequencePlayer->mReadObjects.erase(it_2);
                        }
                    });
    }


    void SequenceEditor::changeMarkerMessage(const std::string &markerID, const std::string &markerMessage)
    {
        performEdit([this, markerID, markerMessage]()
                    {
                        auto it = std::find_if(mSequencePlayer->mSequence->mMarkers.begin(), mSequencePlayer->mSequence->mMarkers.end(), [markerID](
                            ResourcePtr<SequenceMarker> &a) -> bool
                        {
                            return markerID == a->mID;
                        });

                        assert(it != mSequencePlayer->mSequence->mMarkers.end());

                        if(it != mSequencePlayer->mSequence->mMarkers.end())
                        {
                            it->get()->mMessage = markerMessage;
                        }
                    });
    }


    void SequenceEditor::performEdit(std::function<void()> action)
    {
        if(!mPerformingEditAction.load())
        {
            mPerformingEditAction.store(true);
            mSequencePlayer->performEditAction(action);
            mPerformingEditAction.store(false);
        }
    }


    SequenceController *SequenceEditor::getControllerWithTrackType(rtti::TypeInfo trackType)
    {
        auto controller_type = mService.getControllerTypeForTrackType(trackType);
        assert(mControllers.find(controller_type) != mControllers.end()); // entry not found
        return mControllers.find(controller_type)->second.get();
    }
}
