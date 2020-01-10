// local includes
#include "timeline.h"

// external includes
#include <utility/fileutils.h>
#include <rtti/jsonwriter.h>
#include <rtti/jsonreader.h>
#include <rtti/defaultlinkresolver.h>
#include <fstream>

RTTI_BEGIN_CLASS(nap::Timeline)
RTTI_PROPERTY("Name", &nap::Timeline::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Tracks", &nap::Timeline::mTracks, nap::rtti::EPropertyMetaData::Embedded)
RTTI_PROPERTY("Duration", &nap::Timeline::mDuration, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool Timeline::init(utility::ErrorState& errorState)
	{
		if (!Resource::init(errorState))
		{
			return false;
		}

		return true;
	}

	
	bool Timeline::save(const std::string& name, utility::ErrorState& errorState)
	{
		// Ensure the presets directory exists
		const std::string dir = "timelines";
		utility::makeDirs(utility::getAbsolutePath(dir));

		std::string show_path = dir + "/" + name;

		// Serialize current set of parameters to json
		rtti::JSONWriter writer;
		if (!rtti::serializeObjects(rtti::ObjectList{ this }, writer, errorState))
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


	bool Timeline::load(const std::string& name, utility::ErrorState& errorState)
	{
		//
		rtti::DeserializeResult result;

		//
		std::string timelineName = utility::getFileNameWithoutExtension(name);

		//
		std::string timelinePath = name;

		// 
		rtti::Factory factory;
		if (!rtti::readJSONFile(
			timelinePath, 
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
			if (readObject->get_type().is_derived_from<TimelineTrack>())
			{
				mTracks.emplace_back(ResourcePtr<TimelineTrack>(dynamic_cast<TimelineTrack*>(readObject.get())));
			}

			mReadObjects.emplace_back(std::move(readObject));
		}

		return true;
	}
}
