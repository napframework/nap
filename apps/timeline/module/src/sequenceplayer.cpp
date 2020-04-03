// local includes
#include "sequenceplayer.h"
#include "sequenceutils.h"

// nap include
#include <nap/logger.h>

// external includes
#include <utility/fileutils.h>
#include <rtti/jsonwriter.h>
#include <rtti/jsonreader.h>
#include <rtti/defaultlinkresolver.h>
#include <fstream>

RTTI_BEGIN_CLASS(nap::SequencePlayer)
RTTI_PROPERTY("Default Show", &nap::SequencePlayer::mDefaultShow, nap::rtti::EPropertyMetaData::FileLink)
RTTI_PROPERTY("Linked Parameters", &nap::SequencePlayer::mParameters, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Frequency", &nap::SequencePlayer::mFrequency, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool SequencePlayer::init(utility::ErrorState& errorState)
	{
		if (!Resource::init(errorState))
		{
			return false;
		}

		if (!mCreateDefaultShowOnFailure)
		{
			if (errorState.check(load(mDefaultShow, errorState), "Error loading default sequence"))
			{
				return false;
			}
		}
		else if (!load(mDefaultShow, errorState))
		{
			nap::Logger::info(*this, "Error loading default show, creating default sequence based on given parameters");
		
			std::unordered_set<std::string> objectIDs;
			mSequence = sequenceutils::createDefaultSequence(mParameters, mReadObjects, objectIDs);

			nap::Logger::info(*this, "Done creating default sequence, saving it");
			if (errorState.check(!save(mDefaultShow, errorState), "Error saving sequence"))
			{
				return false;
			}
		}

		// launch player thread
		mUpdateThreadRunning = true;
		mUpdateTask = std::async(std::launch::async, std::bind(&SequencePlayer::onUpdate, this));

		return true;
	}


	void SequencePlayer::onDestroy()
	{
		// stop running thread
		mUpdateThreadRunning = false;
		if (mUpdateTask.valid())
		{
			mUpdateTask.wait();
		}
	}


	void SequencePlayer::play()
	{
		std::unique_lock<std::mutex> l = lock();
		mIsPlaying = true;
		mIsPaused = false;
	}


	void SequencePlayer::pause()
	{
		std::unique_lock<std::mutex> l = lock();
		mIsPaused = true;
	}


	void SequencePlayer::stop()
	{
		std::unique_lock<std::mutex> l = lock();
		mIsPlaying = false;
		mIsPaused = false;
	}


	bool SequencePlayer::save(const std::string& name, utility::ErrorState& errorState)
	{
		std::unique_lock<std::mutex> l = lock();

		// Ensure the presets directory exists
		const std::string dir = "sequences";
		utility::makeDirs(utility::getAbsolutePath(dir));

		std::string show_path = name;

		// Serialize current set of parameters to json
		rtti::JSONWriter writer;
		if (!rtti::serializeObjects(rtti::ObjectList{ mSequence }, writer, errorState))
			return false;

		// Open output file
		std::ofstream output(show_path, std::ios::binary | std::ios::out);
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

		//
		std::string timelineName = utility::getFileNameWithoutExtension(name);

		//
		std::string timelinePath = dir + "/" + name;

		// 
		rtti::Factory factory;
		if (!rtti::readJSONFile(
			name,
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

		// if we have loaded the sequence, make a map of assigned parameters id's for each track
		mTrackMap.clear();
		for (const auto& track : mSequence->mTracks)
		{
			for (const auto& parameter : mParameters)
			{
				if (track->mAssignedParameterID == parameter->mID)
				{
					mTrackMap.emplace(parameter->mID, parameter.get());
				}
			}
		}

		return true;
	}


	Sequence& SequencePlayer::getSequence()
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
	
	void SequencePlayer::onUpdate()
	{
		// Compute sleep time in microseconds 
		float sleep_time_microf = 000.0f / static_cast<float>(mFrequency);
		long  sleep_time_micro = static_cast<long>(sleep_time_microf * 1000.0f);

		while (mUpdateThreadRunning)
		{
			{
				std::unique_lock<std::mutex> l = lock();

				//
				std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
				std::chrono::nanoseconds elapsed = now - mBefore;
				float deltaTime = std::chrono::duration<float, std::milli>(elapsed).count() / 1000.0f;
				mBefore = now;

				if (mIsPlaying)
				{
					if (!mIsPaused)
					{
						mTime += deltaTime;
					}

					for (const auto& track : mSequence->mTracks)
					{
						if (mTrackMap.find(track->mAssignedParameterID) != mTrackMap.end())
						{
							auto* parameter = mTrackMap[track->mAssignedParameterID];

							for (const auto& segment : track->mSegments)
							{
								if (mTime > segment->mStartTime &&
									mTime <= segment->mStartTime + segment->mDuration)
								{
									auto value = segment->mCurve->evaluate((mTime - segment->mStartTime) / segment->mDuration);
									parameter->setValue(value);

									break;
								}
							}
						}
					}
				}
			}
			
			std::this_thread::sleep_for(std::chrono::microseconds(sleep_time_micro));
		}
	}


	std::unique_lock<std::mutex> SequencePlayer::lock()
	{
		return std::unique_lock<std::mutex>(mLock);
	}
}
