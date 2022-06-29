/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "sequenceplayer.h"

// nap include
#include <nap/logger.h>
#include <parametervec.h>
#include <parameternumeric.h>

// external includes
#include <utility/fileutils.h>
#include <rtti/jsonwriter.h>
#include <rtti/jsonreader.h>
#include <rtti/defaultlinkresolver.h>
#include <fstream>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequencePlayer)
    RTTI_PROPERTY("Default Show", &nap::SequencePlayer::mSequenceFileName, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Outputs", &nap::SequencePlayer::mOutputs, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("Clock", &nap::SequencePlayer::mClock, nap::rtti::EPropertyMetaData::Embedded);
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
    SequencePlayer::SequencePlayer(SequenceService& service) :
		mService(service)
    {
    }


    bool SequencePlayer::init(utility::ErrorState& errorState)
    {
        if (!Resource::init(errorState))
            return false;

        // Try to load default show
        utility::ErrorState load_error;
        bool loaded = load(mSequenceFileName, load_error);

        // Cancel if can't be loaded and we're not allowed to create an empty sequence on failure
        if (!loaded && !mCreateEmptySequenceOnLoadFail)
        {
            errorState.fail(load_error.toString());
            return false;
        }

        // Otherwise create an empty show if loading fails
        if (!loaded)
        {
            nap::Logger::info(*this, load_error.toString());
            nap::Logger::info(*this, "Unable to load default show, creating default sequence");
            mSequence = mService.createDefaultSequence(mReadObjects, mReadObjectIDs, mOutputs);
            nap::Logger::info(*this, "Done creating default sequence");
        }

        //
        if (!errorState.check(mClock.get()!=nullptr, "Clock cannot be null"))
            return false;

        return true;
    }


    bool SequencePlayer::start(utility::ErrorState& errorState)
    {
        Slot<double> slot(this, &SequencePlayer::tick);
        mClock->start(slot);

        return true;
    }


    void SequencePlayer::stop()
    {
        // stop playing
        setIsPlaying(false);

        // stop clock
        mClock->stop();

        // clear adapters
        destroyAdapters();
    }


    void SequencePlayer::setIsPlaying(bool isPlaying)
    {
        auto lock = std::unique_lock<std::mutex>(mMutex);
        bool was_playing = mIsPlaying;

        if (isPlaying)
        {
            if (!was_playing)
                createAdapters();

            mIsPlaying = true;
            mIsPaused = false;
        }
        else
        {
            destroyAdapters();

            mIsPlaying = false;
            mIsPaused = false;
        }
        lock.unlock();
        playStateChanged(*this, isPlaying);
    }


    void SequencePlayer::setIsPaused(bool isPaused)
    {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mIsPaused = isPaused;
        }
        pauseStateChanged(*this, isPaused);
    }


    bool SequencePlayer::save(const std::string& name, utility::ErrorState& errorState)
    {
        std::lock_guard<std::mutex> lock(mMutex);

        // Ensure the presets directory exists
        const std::string dir = "sequences";
        utility::makeDirs(utility::getAbsolutePath(dir));

        std::string show_path = dir+'/'+name;

        // Serialize current set of parameters to json
        rtti::JSONWriter writer;
        if (!rtti::serializeObjects(rtti::ObjectList{mSequence}, writer, errorState))
            return false;

        // Open output file
        std::ofstream output(show_path, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!errorState.check(output.is_open() && output.good(), "Failed to open %s for writing", show_path.c_str()))
            return false;

        // Write to disk
        std::string json = writer.GetJSON();
        output.write(json.data(), json.size());

        return true;
    }


    bool SequencePlayer::load(const std::string& name, utility::ErrorState& errorState)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        rtti::DeserializeResult result;

        const std::string dir = "sequences";
        utility::makeDirs(utility::getAbsolutePath(dir));
        std::string show_path = dir+'/'+name;

        // Ensure file exists
        if (!errorState.check(!name.empty() && utility::fileExists(show_path), "Show does not exist"))
            return false;

        std::string timeline_name = utility::getFileNameWithoutExtension(name);

        //
        rtti::Factory factory;
        if (!rtti::deserializeJSONFile(show_path, rtti::EPropertyValidationMode::DisallowMissingProperties, rtti::EPointerPropertyMode::NoRawPointers, factory, result, errorState))
            return false;

        // Resolve links
        if (!rtti::DefaultLinkResolver::sResolveLinks(result.mReadObjects, result.mUnresolvedPointers, errorState))
            return false;

        // Move ownership of read objects
        mReadObjects.clear();
        mReadObjectIDs.clear();
        for (auto& read_object : result.mReadObjects)
        {
            if (read_object->get_type().is_derived_from<Sequence>())
            {
                mSequence = static_cast<Sequence*>(read_object.get());
            }

            mReadObjectIDs.emplace(read_object->mID);
            mReadObjects.emplace_back(std::move(read_object));
        }

        // init objects
        for (auto& object_ptr : mReadObjects)
        {
            if (!object_ptr->init(errorState))
                return false;
        }

        // check if we have deserialized a sequence
        if (!errorState.check(mSequence!=nullptr, "sequence is null"))
        {
            return false;
        }

        mSequenceFileName = name;
		sequenceLoaded.trigger(*this, mSequenceFileName);

        // if the sequencer is playing, we need to re-create adapters because assigned outputs probably have changed
        if (mIsPlaying)
        {
            createAdapters();
        }

        return true;
    }


    void SequencePlayer::createAdapters()
    {
        // create adapters
        destroyAdapters();
        for (auto& track : mSequence->mTracks)
        {
            createAdapter(track->mAssignedOutputID, track->mID);
        }

        // add adapter function to be dispatched by signal, in case a custom adapter is to be added
        std::function<void(const std::string&, std::unique_ptr<SequencePlayerAdapter>)> add_adapter_function = [this](
                const std::string& outputID, std::unique_ptr<SequencePlayerAdapter> adapter)
        {
            mAdapters.emplace(outputID, std::move(adapter));
        };

        // dispatch signal with add adapter function
        adaptersCreated.trigger(add_adapter_function);
    }


    void SequencePlayer::destroyAdapters()
    {
        for (auto& pair : mAdapters)
        {
            pair.second->destroy();
        }
        mAdapters.clear();
    }


    Sequence& SequencePlayer::getSequence()
    {
        return *mSequence;
    }


    const Sequence& SequencePlayer::getSequenceConst() const
    {
        return *mSequence;
    }


    double SequencePlayer::getDuration() const
    {
        return mSequence->mDuration;
    }


    void SequencePlayer::setPlayerTime(double time)
    {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mTime = math::clamp<double>(time, 0.0, mSequence->mDuration);
        }
        playerTimeChanged(*this, mTime);
    }


    void SequencePlayer::setPlaybackSpeed(float speed)
    {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mSpeed = speed;
        }
        playbackSpeedChanged(*this, speed);
    }


    double SequencePlayer::getPlayerTime() const
    {
        return mTime;
    }


    bool SequencePlayer::getIsPlaying() const
    {
        return mIsPlaying;
    }


    bool SequencePlayer::getIsPaused() const
    {
        return mIsPaused;
    }


    void SequencePlayer::setIsLooping(bool isLooping)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mIsLooping = isLooping;
    }


    bool SequencePlayer::getIsLooping() const
    {
        return mIsLooping;
    }


    float SequencePlayer::getPlaybackSpeed() const
    {
        return mSpeed;
    }


    void SequencePlayer::tick(double deltaTime)
    {
        if (mIsPlaying)
        {
            // notify lister, so data model of sequence and data of player can be modified by listeners to this signal
            preTick.trigger(*this);

            // Update time and adapters thread safe
            {
                std::lock_guard<std::mutex> lock(mMutex);
                if (!mIsPaused)
                {
                    mTime += deltaTime*(double) mSpeed;
                    if (mIsLooping)
                    {
                        if (mTime<0.0)
                        {
                            mTime = mSequence->mDuration+mTime;
                        }
                        else if (mTime>mSequence->mDuration)
                        {
                            mTime = fmod(mTime, mSequence->mDuration);
                        }
                    }
                    else
                    {
                        mTime = math::clamp<double>(mTime, 0.0, mSequence->mDuration);
                    }
                }

                // Update adapters
                for (auto& adapter : mAdapters)
                    adapter.second->tick(mTime);
            }

            // Notify listeners
            postTick.trigger(*this);
        }
    }


    bool SequencePlayer::createAdapter(const std::string& inputID, const std::string& trackID)
    {
        // bail if empty output id
        if (inputID.empty())
            return false;

        // find track
        SequenceTrack* track = nullptr;

        for (auto& a_track : mSequence->mTracks)
        {
            if (a_track->mID==trackID)
            {
                track = a_track.get();
                break;
            }
        }

        if (track==nullptr)
        {
            nap::Logger::error("No track found with id %s", trackID.c_str());
            return false;
        }

        // erase previous adapter
        if (mAdapters.find(track->mID)!=mAdapters.end())
        {
            mAdapters.erase(track->mID);
        }

        SequencePlayerOutput* output = nullptr;
        for (auto& a_output : mOutputs)
        {
            if (a_output->mID==inputID)
            {
                output = a_output.get();
                break;
            }
        }

        if (output==nullptr)
        {
            // No output found
            // We don't print an error here, because it could be that this track is assigned to a custom output when the
            // adaptersCreated signal is dispatched
            return false;
        }

        auto adapter = mService.invokeAdapterFactory(track->get_type(), *track, *output, *this);

        if (adapter==nullptr)
        {
            nap::Logger::error("Unable to create adapter with track id %s and output id %s", trackID.c_str(), inputID.c_str());
            return false;
        }

        mAdapters.emplace(track->mID, std::move(adapter));

        return true;
    }


    void SequencePlayer::performEditAction(std::function<void()>& action)
    {
		{
			std::lock_guard<std::mutex> lock(mMutex);
			action();
		}
		edited.trigger(*this);
    }


    const std::string& SequencePlayer::getSequenceFilename() const
    {
        return mSequenceFileName;
    }
}
