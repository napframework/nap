#pragma once

// local includes
#include "sequencetracksegmentevent.h"
#include "sequencetrack.h"
#include "sequenceplayerparametersetter.h"
#include "sequencetracksegmentcurve.h"

// external includes
#include <nap/resource.h>
#include <parameter.h>
#include <parametervec.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// forward declares
	class SequencePlayerInput;
	class SequencePlayerAdapter;

	using SequencePlayerAdapterFactoryFunc = std::unique_ptr<SequencePlayerAdapter>(*)(SequenceTrack&, SequencePlayerInput&);

	/**
	 * SequencePlayerAdapter
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
		 * update
		 * called from sequence player thread
		 * @param time time in sequence player
		 */
		virtual void update(double time) = 0;

		static bool registerFactory(rttr::type, SequencePlayerAdapterFactoryFunc factory);

		static std::unique_ptr<SequencePlayerAdapter> invokeFactory(rttr::type type, SequenceTrack& track, SequencePlayerInput& input);
	private:
		static std::unordered_map <rttr::type, SequencePlayerAdapterFactoryFunc>& getFactoryMap();
	};
}