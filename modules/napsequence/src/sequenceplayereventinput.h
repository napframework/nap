#pragma once

// local includes
#include "sequenceplayerinput.h"

// nap includes
#include <nap/resourceptr.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceEventReceiver;
	class SequenceService;

	/**
	 * SequencePlayerEventInput is used to link an SequenceEventReceiver to a SequenceEventTrack
	 */
	class NAPAPI SequencePlayerEventInput : public SequencePlayerInput
	{
		RTTI_ENABLE(SequencePlayerInput);
	public:
		/**
		 * Constructor
		 * @param service reference to SequenceService
		 */
		SequencePlayerEventInput(SequenceService& service);

		ResourcePtr<SequenceEventReceiver> mReceiver; ///< Property: 'Event Receiver' event receiver resource
	protected:
		/**
		 * called from sequence service main thread
		 * @param deltaTime time since last update
		 */
		virtual void update(double deltaTime) override ;
	};

	using SequencePlayerEventInputObjectCreator = rtti::ObjectCreator<SequencePlayerEventInput, SequenceService>;
}