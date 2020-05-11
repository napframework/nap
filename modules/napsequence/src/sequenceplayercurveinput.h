#pragma once

// local includes
#include "sequenceplayerinput.h"
#include "sequenceservice.h"

// nap includes
#include <nap/resourceptr.h>
#include <parameter.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * SequencePlayerCurveInput is used to link a parameter to a curve track
	 */
	class NAPAPI SequencePlayerCurveInput : public SequencePlayerInput
	{
		RTTI_ENABLE(SequencePlayerInput)
	public:
		/**
		 * Constructor
		 * @param service reference to SequenceService
		 */
		SequencePlayerCurveInput(SequenceService& service);
	public:
		// properties
		ResourcePtr<Parameter>	mParameter; ///< Property: 'Parameter' parameter resource
		bool					mUseMainThread; ///< Property: 'Use Main Thread' update in main thread or player thread
	
		// pointer to service
		SequenceService*		mSequenceService; 
	};
}
