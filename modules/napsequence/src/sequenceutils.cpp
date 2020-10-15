// local includes
#include "sequenceutils.h"
#include "sequence.h"
#include "sequencetrackevent.h"
#include "sequencetrackcurve.h"

#include <nap/logger.h>

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace sequenceutils
	{
		std::unordered_map<rttr::type, SequenceDefaultTrackFactoryFunc>& getDefaultFactoryMap()
		{
			static std::unordered_map<rttr::type, SequenceDefaultTrackFactoryFunc> factory;
			return factory;
		}


		bool registerDefaultTrackCreator(rttr::type type, SequenceDefaultTrackFactoryFunc method)
		{
			auto& map = getDefaultFactoryMap();
			auto found_it = map.find(type);
			assert(found_it == map.end()); // method already registered

			map.insert(std::pair<rttr::type, SequenceDefaultTrackFactoryFunc>(type, method));

			return true;
		}


		const std::string generateUniqueID(std::unordered_set<std::string>& objectIDs, const std::string& baseID)
		{
			std::string unique_id = baseID;

			int index = 1;
			while (objectIDs.find(unique_id) != objectIDs.end())
				unique_id = utility::stringFormat("%s_%d", baseID.c_str(), ++index);

			objectIDs.insert(unique_id);

			return unique_id;
		}


		Sequence* createDefaultSequence(std::vector<std::unique_ptr<rtti::Object>>& createdObjects, std::unordered_set<std::string>& objectIDs, const std::vector<ResourcePtr<SequencePlayerOutput>>& outputs)
		{
			// create the sequence
			std::unique_ptr<Sequence> sequence = std::make_unique<Sequence>();
			sequence->mID = sequenceutils::generateUniqueID(objectIDs);
			sequence->mDuration = 1.0;

			// iterate trough the given outputs and see if we can create a default track for the given output
			for(ResourcePtr<SequencePlayerOutput> output : outputs)
			{
				const SequencePlayerOutput* output_ptr = output.get();
				auto& factory_map = getDefaultFactoryMap();
				if(factory_map.find(output_ptr->get_type()) != factory_map.end())
				{
					auto factory_method = factory_map[output_ptr->get_type()];
					std::unique_ptr<SequenceTrack> sequence_track = factory_method(output_ptr);
					sequence_track->mID = sequenceutils::generateUniqueID(objectIDs);
					sequence_track->mAssignedOutputID = output_ptr->mID;
					sequence->mTracks.emplace_back(ResourcePtr<SequenceTrack>(sequence_track.get()));
					createdObjects.emplace_back(std::move(sequence_track));
				}else
				{
					nap::Logger::warn("No factory method found for track output type of %s", output->get_type().get_name().to_string().c_str());
				}
			}

			// store raw pointer to sequence to return
			Sequence* return_ptr = sequence.get();

			// move ownership
			createdObjects.emplace_back(std::move(sequence));

			// finally return
			return return_ptr;
		}
	}
}
