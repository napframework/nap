// local includes
#include "sequenceplayer.h"

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
			mSequence = Sequence::createDefaultSequence(mParameters, mReadObjects, objectIDs);

			nap::Logger::info(*this, "Done creating default sequence, saving it");
			if (errorState.check(!save(mDefaultShow, errorState), "Error saving sequence"))
			{
				return false;
			}
		}

		return true;
	}


	bool SequencePlayer::save(const std::string& name, utility::ErrorState& errorState)
	{
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
		for (auto& readObject : result.mReadObjects)
		{
			//
			if (readObject->get_type().is_derived_from<Sequence>())
			{
				mSequence = dynamic_cast<Sequence*>(readObject.get());
			}

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

		updateDuration();

		return true;
	}


	Sequence& SequencePlayer::getSequence()
	{
		return *mSequence;
	}


	double SequencePlayer::getDuration() const
	{
		return mDuration;
	}


	void SequencePlayer::updateDuration()
	{
		double longestTrack = 0.0;
		for (const auto& trackLink : mSequence->mSequenceTrackLinks)
		{
			double trackTime = 0.0;
			for (const auto& segment : trackLink->mSequenceTrack->mSegments)
			{
				double time = segment->mStartTime + segment->mDuration;
				if (time > trackTime)
				{
					trackTime = time;
				}
			}

			if (trackTime > longestTrack)
			{
				longestTrack = trackTime;
			}
		}

		mDuration = longestTrack;

		mDurationNeedsUpdating = false;
	}
}
