/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "sequenceeditor.h"
#include "sequencetrack.h"
#include "sequencetracksegment.h"
#include "sequenceutils.h"
#include "sequencemarker.h"

// external includes
#include <nap/logger.h>
#include <fcurve.h>
#include <functional>
#include <mathutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceEditor)
RTTI_PROPERTY("Sequence Player", &nap::SequenceEditor::mSequencePlayer, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

using namespace nap::SequenceCurveEnums;

namespace nap
{
	static bool register_object_creator = SequenceService::registerObjectCreator([](SequenceService* service)->std::unique_ptr<rtti::IObjectCreator>
	{
		return std::make_unique<SequencePlayerEditorOutputObjectCreator>(*service);
	});


	SequenceEditor::SequenceEditor(SequenceService& service) : mService(service)
	{

	}


	bool SequenceEditor::init(utility::ErrorState& errorState)
	{
		if (!Resource::init(errorState))
		{
			return false;
		}

		// register editor to service
		mService.registerEditor(*this);

		// create controllers for all types of tracks
		auto& factory = SequenceController::getControllerFactory();
		for (const auto& it : factory)
		{
			mControllers.emplace(it.first, it.second(*mSequencePlayer.get(), *this));
		}

		return true;
	}


	void SequenceEditor::onDestroy()
	{
		mService.removeEditor(*this);
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


	void SequenceEditor::update(double deltaTime)
	{
		if (mPerformingEditAction.load())
		{
			mPerformingEditAction.store(false);
			mSequencePlayer->performEditAction(mEditAction);
		}
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


	SequenceController* SequenceEditor::getControllerWithTrackID(const std::string& trackID)
	{
		const auto& sequence = mSequencePlayer->getSequence();
		for(const auto& track : sequence.mTracks)
		{
			auto track_type = track.get()->get_type();
			auto it = getControllerTrackTypeMap().find(track_type);
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
		queueEditAction([this, file](){
			utility::ErrorState error_state;
			if(!mSequencePlayer->load(file, error_state) )
			{
			  nap::Logger::error(error_state.toString());
			}
		});
	}

	
	void SequenceEditor::changeSequenceDuration(double newDuration)
	{
		newDuration = math::max<double>(newDuration, 0.01);

		queueEditAction([this, newDuration]() {
			auto& sequence = mSequencePlayer->getSequence();

			// sequence must be at least as long as longest track
			double longest_track = 0.0;
			for (auto& track : sequence.mTracks)
			{
				double longest_segment = 0.0;
				for (auto& segment : track->mSegments)
				{
					double time		= segment->mStartTime + segment->mDuration;
					longest_segment = math::max<double>(longest_segment, time);
				}
				longest_track = math::max<double>(longest_segment, longest_track);
			}

			sequence.mDuration = math::max<double>(longest_track, newDuration);
		});

		// reset player position if its bigger then duration
		if( mSequencePlayer->getPlayerTime() > newDuration )
		{
			mSequencePlayer->setPlayerTime(newDuration);
		}
	}


	void SequenceEditor::insertMarker(double time, const std::string& message)
	{
		queueEditAction([this, time, message]() {
			auto new_marker		 = std::make_unique<SequenceMarker>();
			new_marker->mID		 = sequenceutils::generateUniqueID(mSequencePlayer->mReadObjectIDs);
			new_marker->mTime	 = time;
			new_marker->mMessage = message;

			mSequencePlayer->mSequence->mMarkers.emplace_back(ResourcePtr<SequenceMarker>(new_marker.get()));
			mSequencePlayer->mReadObjects.emplace_back(std::move(new_marker));
		});
	}


	void SequenceEditor::changeMarkerTime(const std::string& markerID, double time)
	{
		queueEditAction([this, markerID, time]() {
			auto it =
				std::find_if(mSequencePlayer->mSequence->mMarkers.begin(), mSequencePlayer->mSequence->mMarkers.end(),
							 [markerID](ResourcePtr<SequenceMarker>& a) -> bool { return markerID == a->mID; });

			assert(it != mSequencePlayer->mSequence->mMarkers.end());

			if (it != mSequencePlayer->mSequence->mMarkers.end())
			{
				it->get()->mTime = time;
			}
		});

	}


	void SequenceEditor::deleteMarker(const std::string& markerID)
	{
		queueEditAction([this, markerID]() {
			auto it_1 =
				std::find_if(mSequencePlayer->mSequence->mMarkers.begin(), mSequencePlayer->mSequence->mMarkers.end(),
							 [markerID](ResourcePtr<SequenceMarker>& a) -> bool { return markerID == a->mID; });
			assert(it_1 != mSequencePlayer->mSequence->mMarkers.end());

			if (it_1 != mSequencePlayer->mSequence->mMarkers.end())
			{
				mSequencePlayer->mSequence->mMarkers.erase(it_1);
			}

			auto it_2 =
				std::find_if(mSequencePlayer->mReadObjects.begin(), mSequencePlayer->mReadObjects.end(),
							 [markerID](std::unique_ptr<rtti::Object>& a) -> bool { return markerID == a->mID; });
			assert(it_2 != mSequencePlayer->mReadObjects.end());

			if (it_2 != mSequencePlayer->mReadObjects.end())
			{
				mSequencePlayer->mReadObjects.erase(it_2);
			}
		});
	}


	void SequenceEditor::changeMarkerMessage(const std::string& markerID, const std::string& markerMessage)
	{
		queueEditAction([this, markerID, markerMessage]() {
			auto it =
				std::find_if(mSequencePlayer->mSequence->mMarkers.begin(), mSequencePlayer->mSequence->mMarkers.end(),
							 [markerID](ResourcePtr<SequenceMarker>& a) -> bool { return markerID == a->mID; });

			assert(it != mSequencePlayer->mSequence->mMarkers.end());

			if (it != mSequencePlayer->mSequence->mMarkers.end())
			{
				it->get()->mMessage = markerMessage;
			}
		});
	}


	void SequenceEditor::queueEditAction(std::function<void()> action)
	{
		if (!mPerformingEditAction.load())
		{
			mPerformingEditAction.store(true);
			mEditAction = std::move(action);
		}else
		{
			nap::Logger::warn(*this, "adding edit action while still waiting for previous action to be executed!");
		}
	}
}
