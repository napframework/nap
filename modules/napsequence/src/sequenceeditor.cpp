// local includes
#include "sequenceeditor.h"
#include "sequencetrack.h"
#include "sequencetracksegment.h"
#include "sequenceutils.h"

// external includes
#include <nap/logger.h>
#include <fcurve.h>
#include <functional>

RTTI_BEGIN_CLASS(nap::SequenceEditor)
RTTI_PROPERTY("Sequence Player", &nap::SequenceEditor::mSequencePlayer, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

using namespace nap::SequenceCurveEnums;

namespace nap
{
	bool SequenceEditor::init(utility::ErrorState& errorState)
	{
		if (!Resource::init(errorState))
		{
			return false;
		}

		// create controllers for all types of tracks
		auto& factory = SequenceController::getControllerFactory();
		for (auto it : factory)
		{
			mControllers.emplace(it.first, it.second(*mSequencePlayer.get()));
		}

		return true;
	}

	std::unordered_map<rttr::type, rttr::type>& getControllerTrackTypeMap()
	{
		static std::unordered_map<rttr::type, rttr::type> map;
		return map;
	}


	bool SequenceEditor::registerControllerForTrackType(rttr::type trackType, rttr::type controllerType)
	{
		auto& map = getControllerTrackTypeMap();
		assert(map.find(controllerType) == map.end()); // duplicate entry
		if (map.find(controllerType) == map.end())
		{
			map.emplace(trackType, controllerType);
			return true;
		}

		return false;
	}


	SequenceController* SequenceEditor::getControllerWithTrackType(rttr::type type)
	{
		auto& map = getControllerTrackTypeMap();
		if (map.find(type) != map.end())
		{
			auto it = getControllerTrackTypeMap().find(type);

			if (mControllers.find(it->second) != mControllers.end())
			{
				return mControllers[it->second].get();
			}
		}
		
		return nullptr;
	}


	void SequenceEditor::save(const std::string& file)
	{
		utility::ErrorState error_state;
		if(!mSequencePlayer->save(file, error_state))
		{
			nap::Logger::error(error_state.toString());
		}
	}


	void SequenceEditor::load(const std::string& file)
	{
		utility::ErrorState error_state;
		if(!mSequencePlayer->load(file, error_state) )
		{
			nap::Logger::error(error_state.toString());
		}
	}


	double SequenceEditor::getDuration()
	{
		return mSequencePlayer->mSequence->mDuration;
	}
}
