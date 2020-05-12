#pragma once

// local includes
#include "sequencetrack.h"

// external includes
#include <nap/resource.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequencePlayerInput;
	class SequencePlayerAdapter;

	// shortcut to factory function
	using SequencePlayerAdapterFactoryFunc = std::unique_ptr<SequencePlayerAdapter>(*)(SequenceTrack&, SequencePlayerInput&);

	/**
	 * A SequencePlayerAdapter can be created by the SequencePlayer and syncs with the player thread
	 * Typically, a SequencePlayerAdapter is responsible for doing something with a track while the player is playing
	 */
	class SequencePlayerAdapter
	{
	public:
		/**
		 * Constructor
		 */
		SequencePlayerAdapter() {};

		/**
		 * Deconstructor
		 */
		virtual ~SequencePlayerAdapter() {}

		/**
		 * called from sequence player thread
		 * @param time time in sequence player
		 */
		virtual void update(double time) = 0;

		/**
		 * registers factory method for specific track type
		 * @param type the type of track that is associated with the factory method
		 * @param factory the factory method
		 * @return true if registration is successful
		 */
		static bool registerFactory(rttr::type type, SequencePlayerAdapterFactoryFunc factory);

		/**
		 * Invokes factory method and returns unique ptr to created adapter, nullptr when not successfull
		 * @param type track type
		 * @param track reference to track
		 * @param input reference to input
		 * @return unique ptr to created adapter, nullptr upon failure
		 */
		static std::unique_ptr<SequencePlayerAdapter> invokeFactory(rttr::type type, SequenceTrack& track, SequencePlayerInput& input);
	private:
		// returns factory map
		static std::unordered_map <rttr::type, SequencePlayerAdapterFactoryFunc>& getFactoryMap();
	};
}