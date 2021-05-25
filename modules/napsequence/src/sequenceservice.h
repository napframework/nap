/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "sequenceplayer.h"

// External Includes
#include <nap/service.h>
#include <entity.h>
#include <nap/datetime.h>
#include <rtti/factory.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceController;
	class SequenceEditor;
	class SequencePlayerAdapter;

	// shortcuts
	using SequenceControllerFactoryFunc 	= std::function<std::unique_ptr<SequenceController>(SequencePlayer&, SequenceEditor&)>;
	using SequenceControllerFactoryMap 		= std::unordered_map<rtti::TypeInfo, SequenceControllerFactoryFunc>;
	using DefaultSequenceTrackFactoryMap 	= std::unordered_map<rtti::TypeInfo, std::function<std::unique_ptr<SequenceTrack>(const SequencePlayerOutput*)>>;
	using SequencePlayerAdapterFactoryFunc 	= std::function<std::unique_ptr<SequencePlayerAdapter>(const SequenceTrack&, SequencePlayerOutput&, const SequencePlayer&)>;
	using SequencePlayerAdapterFactoryMap 	= std::unordered_map<rtti::TypeInfo, SequencePlayerAdapterFactoryFunc>;

	/**
	 * SequenceService is responsible for updating outputs and contains information about which controller, adapter is
	 * used for editing registered track types. When adding a new track type, make sure to register the new controller and
	 * adapter factory methods by calling the appropriate register functions on the SequenceService.
	 * A good place to do this from is from your own (app) Module Service that will always initialize itself before any
	 * SequenceEditor resource
	 */
	class NAPAPI SequenceService final : public Service
	{
		friend class SequencePlayerOutput;

		RTTI_ENABLE(Service)
	public:
		/**
		 * Constructor
		 */
		SequenceService(ServiceConfiguration* configuration);

		/**
		 * Deconstructor
		 */
		~SequenceService() override;

		/**
		 * can be used to register a default creation method to the factory. When createDefaultSequence is called, it will iterate trough the given outputs and
		 * create a SequenceTrack that fits the output. Whenever we create a new type of output, we should also add a way to create a default track
		 * @param type the type information of the sequence output
		 * @param method the factory method
		 * @return true on successful creation
		 */
		bool registerDefaultTrackCreatorForOutput(rtti::TypeInfo outputType,
												  std::function<std::unique_ptr<SequenceTrack>(const SequencePlayerOutput*)> func);

		/**
		 * creates a default sequence based on given outputs
		 * @param createdObjects a reference to a vector that will be filled with unique pointers of created objects
		 * @param objectIDs a list of unique ids, used to created unique ids for each object in this sequence
		 * @param outputs a list of player outputs
		 * @return a raw pointer to the newly created sequence, ownership of sequence is stored as a unique pointer in createdObjects
		 */
		Sequence* createDefaultSequence(std::vector<std::unique_ptr<rtti::Object>>& createdObjects,
										std::unordered_set<std::string>& objectIDs,
										const std::vector<ResourcePtr<SequencePlayerOutput>>& outputs);

		/**
		 * Method that registers a certain controller type for a certain view type, this can be used by views to map controller types to view types
		 * @param viewType the viewtype
		 * @param controllerType the controller type
		 * @return true on successful registration
		 */
		bool registerControllerForTrackType(rtti::TypeInfo viewType, rtti::TypeInfo controllerType);

		/**
		 * returns controller type info for give track type
		 * @param trackType the track type
		 * @return the controller type
		 */
		rtti::TypeInfo getControllerTypeForTrackType(rtti::TypeInfo trackType);

		/**
		 * registers a new controller factory function for controller type
		 * @param controllerType the type of controller for which to register a new factory function
		 * @return true on success
		 */
		bool registerControllerFactoryFunc(rtti::TypeInfo controllerType, SequenceControllerFactoryFunc);

		/**
		 * registers adapter factory method for specific track type
		 * @param type the type of track that is associated with the factory method
		 * @param factory the factory method
		 * @return true if registration is successful
		 */
		bool registerAdapterFactoryFunc(rtti::TypeInfo typeInfo, SequencePlayerAdapterFactoryFunc factory);

		/**
		 * Invokes adapter factory method and returns unique ptr to created adapter, assert when track type not found
		 * @param type track type
		 * @param track reference to track
		 * @param output reference to input
		 * @param player sequence player creating adapter
		 * @return unique ptr to created adapter, nullptr upon failure
		 */
		std::unique_ptr<SequencePlayerAdapter> invokeAdapterFactory(rtti::TypeInfo type, const SequenceTrack& track,
																	SequencePlayerOutput& output,
																	const SequencePlayer& player);

		/**
		 * Invokes controller factory method and returns unique_ptr to controller, asserts when controller type is not found
		 * @param controllerType the type of controller to create
		 * @param player reference to player
		 * @param editor reference to editor
		 * @return unique_ptr to controller
		 */
		std::unique_ptr<SequenceController> invokeControllerFactory(rtti::TypeInfo controllerType,
																	SequencePlayer& player,
																	SequenceEditor& editor);

		/**
		 * returns all registered controller types that have registered factory functions
		 * @return vector of controller type info
		 */
		std::vector<rtti::TypeInfo> getRegisteredControllerTypes() const;

		/**
		 * generate a unique id
		 * @param objectIDs reference to collections of id's
		 * @param baseID base id
		 * @return unique id
		 */
		std::string generateUniqueID(std::unordered_set<std::string>& objectIDs, const std::string& baseID = "Generated");

		/**
		 * registers object creator method that can be passed on to the rtti factory
		 * @param objectCreator unique pointer to method
		 */
		static bool registerObjectCreator(std::unique_ptr<rtti::IObjectCreator>(*objectCreator)(SequenceService*));
	protected:
		/**
		 * registers all objects that need a specific way of construction
		 * @param factory the factory to register the object creators with
		 */
		void registerObjectCreators(rtti::Factory& factory) override;

		/**
		 * initializes service
		 * @param errorState contains any errors
		 * @return returns true on successful initialization
		 */
		bool init(nap::utility::ErrorState& errorState) override;

		/**
		 * updates any outputs and editors
		 * @param deltaTime deltaTime
		 */
		void update(double deltaTime) override;
	private:
		/**
		 * registers an output
		 * @param output reference to output
		 */
		void registerOutput(SequencePlayerOutput& output);

		/**
		 * removes an output
		 * @param output reference to output
		 */
		void removeOutput(SequencePlayerOutput& output);

		// vector holding raw pointers to outputs
		std::vector<SequencePlayerOutput*> mOutputs;

		// factory map for default creation of tracks
		DefaultSequenceTrackFactoryMap mDefaultTrackCreatorMap;

		// map of controller type information for each track type
		std::unordered_map<rtti::TypeInfo, rtti::TypeInfo> mControllerTypesTrackTypeMap;

		// map of controller factory functions
		SequenceControllerFactoryMap mControllerFactory;

		// map of sequence player adapter factory functions
		SequencePlayerAdapterFactoryMap mAdapterFactory;
	};
}
