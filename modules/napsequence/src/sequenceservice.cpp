/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <iostream>
#include <utility/stringutils.h>
#include <parametervec.h>
#include <parameternumeric.h>

// Local Includes
#include "sequenceplayeradapter.h"
#include "sequenceplayeroutput.h"
#include "sequenceservice.h"
#include "sequenceeditor.h"
#include "sequencetrackevent.h"
#include "sequenceplayercurveoutput.h"
#include "sequencetrackcurve.h"
#include "sequenceplayereventoutput.h"
#include "sequenceplayer.h"
#include "sequencecontrollercurve.h"
#include "sequencecontrollerevent.h"
#include "sequencecontroller.h"
#include "sequenceplayereventadapter.h"
#include "sequenceplayercurveadapter.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceService)
RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	static std::vector<std::unique_ptr<rtti::IObjectCreator>(*)(SequenceService*)>& getObjectCreators()
	{
		static std::vector<std::unique_ptr<rtti::IObjectCreator>(*)(SequenceService* service)> vector;
		return vector;
	}


	bool SequenceService::registerObjectCreator(std::unique_ptr<rtti::IObjectCreator>(*objectCreator)(SequenceService* service))
	{
		getObjectCreators().emplace_back(objectCreator);
		return true;
	}


	SequenceService::SequenceService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


	SequenceService::~SequenceService() = default;


	void SequenceService::registerObjectCreators(rtti::Factory& factory)
	{
		for(auto& objectCreator : getObjectCreators())
		{
			factory.addObjectCreator(objectCreator(this));
		}

		factory.addObjectCreator(std::make_unique<SequencePlayerObjectCreator>(*this));
		factory.addObjectCreator(std::make_unique<SequenceEditorObjectCreator>(*this));
		factory.addObjectCreator(std::make_unique<SequencePlayerEventOutputObjectCreator>(*this));
		factory.addObjectCreator(std::make_unique<SequencePlayerCurveOutputObjectCreator>(*this));
	}


	bool SequenceService::init(nap::utility::ErrorState& errorState)
	{
		// register the default track creation method for curved outputs
		if(!errorState.check(registerDefaultTrackCreatorForOutput( RTTI_OF(SequencePlayerCurveOutput),
																  [](const SequencePlayerOutput* output) -> std::unique_ptr<SequenceTrack>
			{
				assert(RTTI_OF(SequencePlayerCurveOutput) == output->get_type()); // type mismatch

				// cast the output to a curve output
				const auto* curve_output = static_cast<const SequencePlayerCurveOutput*>(output);

				// check the parameter
				assert(curve_output->mParameter != nullptr); // parameter must be assigned

				// declare return ptr
				std::unique_ptr<SequenceTrack> sequence_track = nullptr;

				// check what type of parameter is being used and create a track that fits the parameter
				// ParameterVec2 = SequenceTrackCurveVec2
				// ParameterVec3 = SequenceTrackCurveVec3
				// ParameterFloat, ParameterLong, ParameterInt & ParameterDouble = SequenceTrackCurveFloat
				if (curve_output->mParameter.get()->get_type() == RTTI_OF(ParameterVec2))
				{
				  sequence_track = std::make_unique<SequenceTrackCurveVec2>();
				}
				else if (curve_output->mParameter.get()->get_type() == RTTI_OF(ParameterVec3))
				{
				  sequence_track = std::make_unique<SequenceTrackCurveVec3>();
				}else if( 	curve_output->mParameter.get()->get_type() == RTTI_OF(ParameterFloat) ||
						   curve_output->mParameter.get()->get_type() == RTTI_OF(ParameterLong) ||
						   curve_output->mParameter.get()->get_type() == RTTI_OF(ParameterInt) ||
						   curve_output->mParameter.get()->get_type() == RTTI_OF(ParameterDouble) )
				{
				  sequence_track = std::make_unique<SequenceTrackCurveFloat>();
				}

				assert(sequence_track != nullptr); // couldn't create default track with parameter type
				return sequence_track;
			}), "Error registering default track creator"))
				return false;

		// register default track creator for event outputs
		if(!errorState.check(registerDefaultTrackCreatorForOutput( RTTI_OF(SequencePlayerEventOutput),
																   [](const SequencePlayerOutput* output) -> std::unique_ptr<SequenceTrack>
			{
				return std::make_unique<SequenceTrackEvent>();
			}), "Error registering default track creator"))
				return false;

		// register the same controller for different curved track types
		if( !registerControllerForTrackType(RTTI_OF(SequenceTrackCurveFloat), RTTI_OF(SequenceControllerCurve)) ||
			!registerControllerForTrackType(RTTI_OF(SequenceTrackCurveVec2), RTTI_OF(SequenceControllerCurve)) ||
			!registerControllerForTrackType(RTTI_OF(SequenceTrackCurveVec3), RTTI_OF(SequenceControllerCurve)) ||
			!registerControllerForTrackType(RTTI_OF(SequenceTrackCurveVec4), RTTI_OF(SequenceControllerCurve)))
		{
			errorState.fail("Error registering curve controller types");
			return false;
		}

		// register the event controller type
		if(!registerControllerForTrackType(RTTI_OF(SequenceTrackEvent), RTTI_OF(SequenceControllerEvent)))
		{
			errorState.fail("Error registering event controller type");
			return false;
		}

		// register the curve controller factory function
		if(	!errorState.check(registerControllerFactoryFunc(
				RTTI_OF(SequenceControllerCurve),
				[this](SequencePlayer& player, SequenceEditor& editor)->std::unique_ptr<SequenceController>
				{
					return std::make_unique<SequenceControllerCurve>(*this, player, editor);
				}), "Error registering controller factory function"))
					return false;

		// register the event controller factory function
		if(	!errorState.check(registerControllerFactoryFunc(
				RTTI_OF(SequenceControllerEvent),
				[this](SequencePlayer& player, SequenceEditor& editor)->std::unique_ptr<SequenceController>
				{
				  	return std::make_unique<SequenceControllerEvent>(*this, player, editor);
				}), "Error registering controller factory function"))
					return false;

		/**
		 * register the adapter factory functions for event tracks & curved tracks
		 */

		// event track adapter factory function
		if(!errorState.check(registerAdapterFactoryFunc(RTTI_OF(SequenceTrackEvent),
										[](const SequenceTrack& track, SequencePlayerOutput& output, const SequencePlayer& player)->std::unique_ptr<SequencePlayerAdapter>
			{
				assert(output.get_type() == RTTI_OF(SequencePlayerEventOutput)); // type mismatch

				auto& eventOutput = *rtti_cast<SequencePlayerEventOutput>(&output);

				auto adapter = std::make_unique<SequencePlayerEventAdapter>(track, eventOutput, player);
				return std::move(adapter);
			}), "Error registering adapter factory function"))
				return false;

		// curve float adapter factory function
		if(!errorState.check(registerAdapterFactoryFunc(RTTI_OF(SequenceTrackCurveFloat),
											[](const SequenceTrack& track, SequencePlayerOutput& output, const SequencePlayer& player)->std::unique_ptr<SequencePlayerAdapter>
			{
				assert(track.get_type() == RTTI_OF(SequenceTrackCurveFloat)); // type mismatch
				assert(output.get_type() == RTTI_OF(SequencePlayerCurveOutput)); //  type mismatch

				auto& curve_output = static_cast<SequencePlayerCurveOutput&>(output);

				if (curve_output.mParameter.get()->get_type() == RTTI_OF(ParameterFloat))
				{
				  return std::make_unique<SequencePlayerCurveAdapter<float, ParameterFloat, float>>(track, curve_output);
				}

				if (curve_output.mParameter.get()->get_type() == RTTI_OF(ParameterLong))
				{
				  return std::make_unique<SequencePlayerCurveAdapter<float, ParameterLong, long>>(track, curve_output);
				}

				if (curve_output.mParameter.get()->get_type() == RTTI_OF(ParameterDouble))
				{
				  return std::make_unique<SequencePlayerCurveAdapter<float, ParameterDouble, double>>(track, curve_output);
				}

				if (curve_output.mParameter.get()->get_type() == RTTI_OF(ParameterInt))
				{
				  	return std::make_unique<SequencePlayerCurveAdapter<float, ParameterInt, int>>(track, curve_output);
				}

				assert(false); // no correct parameter type found!
				return nullptr;
			}), "Error registering adapter factory function"))
				return false;

		// vec2 curve adapter factory function
		if(!errorState.check(registerAdapterFactoryFunc(RTTI_OF(SequenceTrackCurveVec2),
										[](const SequenceTrack& track, SequencePlayerOutput& output, const SequencePlayer& player)->std::unique_ptr<SequencePlayerAdapter>
			{
				assert(track.get_type() == RTTI_OF(SequenceTrackCurveVec2)); // type mismatch
				assert(output.get_type() == RTTI_OF(SequencePlayerCurveOutput)); //  type mismatch

				auto& curve_output = static_cast<SequencePlayerCurveOutput&>(output);

				assert(curve_output.mParameter.get()->get_type() == RTTI_OF(ParameterVec2)); // type mismatch
				if (curve_output.mParameter.get()->get_type() == RTTI_OF(ParameterVec2))
				{
				  return std::make_unique<SequencePlayerCurveAdapter<glm::vec2, ParameterVec2, glm::vec2>>(track, curve_output);
				}

				return nullptr;
			}), "Error registering adapter factory function"))
				return false;

		// vec3 adapter factory function
		if(!errorState.check(registerAdapterFactoryFunc(RTTI_OF(SequenceTrackCurveVec3),
										[](const SequenceTrack& track, SequencePlayerOutput& output, const SequencePlayer& player)->std::unique_ptr<SequencePlayerAdapter>
			{
				assert(track.get_type() == RTTI_OF(SequenceTrackCurveVec3)); // type mismatch
				assert(output.get_type() == RTTI_OF(SequencePlayerCurveOutput)); //  type mismatch

				auto& curve_output = static_cast<SequencePlayerCurveOutput&>(output);

				assert(curve_output.mParameter.get()->get_type() == RTTI_OF(ParameterVec3)); // type mismatch
				if (curve_output.mParameter.get()->get_type() == RTTI_OF(ParameterVec3))
				{
				  return std::make_unique<SequencePlayerCurveAdapter<glm::vec3, ParameterVec3, glm::vec3>>(track, curve_output);
				}

				return nullptr;
			}), "Error registering adapter factory function"))
				return false;

		// vec4 adapter factory function
		if(!errorState.check(registerAdapterFactoryFunc(RTTI_OF(SequenceTrackCurveVec4),
										[](const SequenceTrack& track, SequencePlayerOutput& output, const SequencePlayer& player)->std::unique_ptr<SequencePlayerAdapter>
			{
				nap::Logger::info("adapter not yet implemented!");
				return nullptr;
			}), "Error registering adapter factory function"))
				return false;

		return true;
	}


	Sequence* SequenceService::createDefaultSequence(std::vector<std::unique_ptr<rtti::Object>>& createdObjects, std::unordered_set<std::string>& objectIDs, const std::vector<ResourcePtr<SequencePlayerOutput>>& outputs)
	{
		// create the sequence
		std::unique_ptr<Sequence> sequence = std::make_unique<Sequence>();
		sequence->mID = generateUniqueID(objectIDs);
		sequence->mDuration = 1.0;

		// iterate trough the given outputs and see if we can create a default track for the given output
		for(ResourcePtr<SequencePlayerOutput> output : outputs)
		{
			const SequencePlayerOutput* output_ptr = output.get();
			if(mDefaultTrackCreatorMap.find(output_ptr->get_type()) != mDefaultTrackCreatorMap.end())
			{
				auto factory_method = mDefaultTrackCreatorMap[output_ptr->get_type()];
				std::unique_ptr<SequenceTrack> sequence_track = factory_method(output_ptr);
				sequence_track->mID = generateUniqueID(objectIDs);
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


	void SequenceService::update(double deltaTime)
	{
		for(auto& output : mOutputs)
		{
			output->update(deltaTime);
		}
	}


	void SequenceService::registerOutput(SequencePlayerOutput& input)
	{
		auto found_it = std::find_if(mOutputs.begin(), mOutputs.end(), [&](const auto& it)
		{
			return it == &input;
		});
		assert(found_it == mOutputs.end()); // duplicate entry

		if(found_it == mOutputs.end())
		{
			mOutputs.emplace_back(&input);
		}
	}


	void SequenceService::removeOutput(SequencePlayerOutput& input)
	{
		auto found_it = std::find_if(mOutputs.begin(), mOutputs.end(), [&](const auto& it)
		{
			return it == &input;
		});

		if(found_it != mOutputs.end())
		{
			mOutputs.erase(found_it);
		}
	}


	bool SequenceService::registerDefaultTrackCreatorForOutput(rtti::TypeInfo outputType,
															   std::function<std::unique_ptr<SequenceTrack>(const SequencePlayerOutput*)> func)
	{
		assert(mDefaultTrackCreatorMap.find(outputType)==mDefaultTrackCreatorMap.end()); // duplicate entry
		return mDefaultTrackCreatorMap.emplace(outputType, func).second;
	}


	bool SequenceService::registerControllerForTrackType(rtti::TypeInfo trackType, rtti::TypeInfo controllerType)
	{
		assert(mControllerTypesTrackTypeMap.find(trackType)==mControllerTypesTrackTypeMap.end()); // duplicate entry
		return mControllerTypesTrackTypeMap.emplace(trackType, controllerType).second;
	}


	rtti::TypeInfo SequenceService::getControllerTypeForTrackType(rtti::TypeInfo trackType)
	{
		assert(mControllerTypesTrackTypeMap.find(trackType)!=mControllerTypesTrackTypeMap.end()); // entry not found
		return mControllerTypesTrackTypeMap.find(trackType)->second;
	}


	std::unique_ptr<SequenceController> SequenceService::invokeControllerFactory(rtti::TypeInfo controllerType,
																				 SequencePlayer& player,
																				 SequenceEditor& editor)
	{
		assert(mControllerFactory.find(controllerType)!=mControllerFactory.end());
		return mControllerFactory.find(controllerType)->second(player, editor);
	}


	std::vector<rtti::TypeInfo> SequenceService::getRegisteredControllerTypes() const
	{
		std::vector<rtti::TypeInfo> controller_types;
		for(const auto& entry : mControllerFactory)
		{
			controller_types.emplace_back(entry.first);
		}
		return controller_types;
	}


	bool SequenceService::registerControllerFactoryFunc(rtti::TypeInfo controllerType, SequenceControllerFactoryFunc func)
	{
		assert(mControllerFactory.find(controllerType)==mControllerFactory.end()); // entry not found
		return mControllerFactory.emplace(controllerType, func).second;
	}


	bool SequenceService::registerAdapterFactoryFunc(rtti::TypeInfo typeInfo, SequencePlayerAdapterFactoryFunc func)
	{
		assert(mAdapterFactory.find(typeInfo)==mAdapterFactory.end()); // duplicate entry
		return mAdapterFactory.emplace(typeInfo, func).second;
	}


	std::unique_ptr<SequencePlayerAdapter> SequenceService::invokeAdapterFactory(rtti::TypeInfo type, const SequenceTrack& track, SequencePlayerOutput& output, const SequencePlayer& player)
	{
		assert(mAdapterFactory.find(type)!=mAdapterFactory.end()); // entry not found
		return mAdapterFactory[type](track, output, player);
	}


	std::string SequenceService::generateUniqueID(std::unordered_set<std::string>& objectIDs, const std::string& baseID)
	{
		std::string unique_id = baseID;

		int index = 1;
		while (objectIDs.find(unique_id) != objectIDs.end())
			unique_id = utility::stringFormat("%s_%d", baseID.c_str(), ++index);

		objectIDs.insert(unique_id);

		return unique_id;
	}
}