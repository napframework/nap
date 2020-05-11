// local includes
#include "sequenceplayer.h"
#include "sequenceutils.h"

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
RTTI_PROPERTY("Inputs", &nap::SequencePlayer::mInputs, nap::rtti::EPropertyMetaData::Embedded)
RTTI_PROPERTY("Frequency", &nap::SequencePlayer::mFrequency, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	SequencePlayer::SequencePlayer(SequenceService& service)
		:mSequenceService(service)
	{
	}


	bool SequencePlayer::init(utility::ErrorState& errorState)
	{
		if (!Resource::init(errorState))
		{
			return false;
		}

		if (!mCreateEmptySequenceOnLoadFail)
		{
			if (errorState.check(load(mSequenceFileName, errorState), "Error loading default sequence"))
			{
				return false;
			}
		}
		else if (!load(mSequenceFileName, errorState))
		{
			nap::Logger::info(*this, errorState.toString());
			nap::Logger::info(*this, "Error loading default show, creating default sequence");

			mSequence = sequenceutils::createEmptySequence(mReadObjects, mReadObjectIDs);

			nap::Logger::info(*this, "Done creating default sequence");
		}

		return true;
	}


	bool SequencePlayer::start(utility::ErrorState& errorState)
	{
		// launch player thread
		mUpdateThreadRunning = true;
		mUpdateTask = std::async(std::launch::async, std::bind(&SequencePlayer::onUpdate, this));

		return true;
	}


	void SequencePlayer::stop()
	{
		// stop running thread
		mUpdateThreadRunning = false;
		if (mUpdateTask.valid())
		{
			mUpdateTask.wait();
		}
	}


	void SequencePlayer::setIsPlaying(bool isPlaying)
	{
		std::unique_lock<std::mutex> l = lock();

		if (isPlaying)
		{
			mIsPlaying = true;
			mIsPaused = false;
		}
		else
		{
			mIsPlaying = false;
			mIsPaused = false;
		}
	}


	void SequencePlayer::setIsPaused(bool isPaused)
	{
		std::unique_lock<std::mutex> l = lock();
		mIsPaused = isPaused;
	}


	bool SequencePlayer::save(const std::string& name, utility::ErrorState& errorState)
	{
		std::unique_lock<std::mutex> l = lock();

		// Ensure the presets directory exists
		const std::string dir = "sequences";
		utility::makeDirs(utility::getAbsolutePath(dir));

		std::string show_path = dir + '/' + name;

		// Serialize current set of parameters to json
		rtti::JSONWriter writer;
		if (!rtti::serializeObjects(rtti::ObjectList{ mSequence }, writer, errorState))
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
		std::unique_lock<std::mutex> l = lock();

		//
		rtti::DeserializeResult result;

		const std::string dir = "sequences";
		utility::makeDirs(utility::getAbsolutePath(dir));
		std::string show_path = dir + '/' + name;

		//
		std::string timelineName = utility::getFileNameWithoutExtension(name);

		// 
		rtti::Factory factory;
		if (!rtti::readJSONFile(
			show_path,
			rtti::EPropertyValidationMode::DisallowMissingProperties,
			rtti::EPointerPropertyMode::NoRawPointers,
			factory,
			result,
			errorState))
			return false;

		// Resolve links
		if (!rtti::DefaultLinkResolver::sResolveLinks(result.mReadObjects, result.mUnresolvedPointers, errorState))
			return false;

		// Move ownership of read objects
		mReadObjects.clear();
		mReadObjectIDs.clear();
		for (auto& readObject : result.mReadObjects)
		{
			//
			if (readObject->get_type().is_derived_from<Sequence>())
			{
				mSequence = dynamic_cast<Sequence*>(readObject.get());
			}

			mReadObjectIDs.emplace(readObject->mID);
			mReadObjects.emplace_back(std::move(readObject));
		}

		// init objects
		for (auto& objectPtr : mReadObjects)
		{
			if (!objectPtr->init(errorState))
				return false;
		}

		// check if we have deserialized a sequence
		if (errorState.check(mSequence == nullptr, "sequence is null"))
		{
			return false;
		}

		// create adapters
		mAdapters.clear();
		for (auto& track : mSequence->mTracks)
		{
			createAdapter(track->mAssignedInputID, track->mID, l);
		}

		mSequenceFileName = name;

		return true;
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
		std::unique_lock<std::mutex> l = lock();

		mTime = time;
		mTime = math::clamp<double>(mTime, 0.0, mSequence->mDuration);
	}


	void SequencePlayer::setPlaybackSpeed(float speed)
	{
		std::unique_lock<std::mutex> l = lock();

		mSpeed = speed;
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
		std::unique_lock<std::mutex> l = lock();

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

	
	void SequencePlayer::onUpdate()
	{
		// Compute sleep time in microseconds 
		float sleep_time_microf = 000.0f / static_cast<float>(mFrequency);
		long  sleep_time_micro = static_cast<long>(sleep_time_microf * 1000.0f);

		while (mUpdateThreadRunning)
		{
			// stack push for lock
			{
				// lock
				std::unique_lock<std::mutex> l = lock();

				// advance time
				std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
				std::chrono::nanoseconds elapsed = now - mBefore;
				float deltaTime = std::chrono::duration<float, std::milli>(elapsed).count() / 1000.0f;
				mBefore = now;

				//
				if (mIsPlaying)
				{
					if (!mIsPaused)
					{
						mTime += deltaTime * mSpeed;

						if (mIsLooping)
						{
							if (mTime < 0.0)
							{
								mTime = mSequence->mDuration + mTime;
							}
							else if (mTime > mSequence->mDuration)
							{
								mTime = fmod(mTime, mSequence->mDuration);
							}
						}
						else
						{
							mTime = math::clamp<double>(mTime, 0.0, mSequence->mDuration);
						}
					}

					for (auto& adapter : mAdapters)
					{
						adapter.second->update(mTime);
					}
				}
			}
			
			std::this_thread::sleep_for(std::chrono::microseconds(sleep_time_micro));
		}
	}


	bool SequencePlayer::createAdapter(const std::string& inputID, const std::string& trackID, const std::unique_lock<std::mutex>& l)
	{
		// bail if empty input id
		if (inputID == "")
			return false;

		// find track
		SequenceTrack* track = nullptr;

		for (auto& aTrack : mSequence->mTracks)
		{
			if (aTrack->mID == trackID)
			{
				track = aTrack.get();
				break;
			}
		}

		assert(track != nullptr); // no track found with id
		if (track == nullptr)
		{
			nap::Logger::error("No track found with id %s", trackID.c_str());
			return false;
		}

		// erase previous adapter
		if (mAdapters.find(track->mID) != mAdapters.end())
		{
			mAdapters.erase(track->mID);
		}

		SequencePlayerInput* input = nullptr;
		for (auto& aInput : mInputs)
		{
			if (aInput->mID == inputID)
			{
				input = aInput.get();
				break;
			}
		}

		assert(input != nullptr); // no input found with id
		if (input == nullptr)
		{
			nap::Logger::error("No input found with id %s", inputID.c_str());
			return false;
		}

		auto adapter = SequencePlayerAdapter::invokeFactory(track->get_type(), *track, *input);

		assert(adapter != nullptr); // unable to create adapter
		if (adapter == nullptr)
		{
			nap::Logger::error("Unable to create adapter with track id %s and input id %s", trackID.c_str(), inputID.c_str());
			return false;
		}

		mAdapters.emplace(track->mID, std::move(adapter));

		return true;
	}


	std::unique_lock<std::mutex> SequencePlayer::lock()
	{
		return std::unique_lock<std::mutex>(mLock);
	}
}
