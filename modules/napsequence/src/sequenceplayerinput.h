#pragma once

// nap includes
#include <nap/resource.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * SequencePlayerInput is the base class for inputs for the sequenceplayer
	 * Inputs can be used by SequencePlayerAdapters to link tracks to objects.
	 * F.E SequencePlayerCurveInput can link a parameter to a curve track
	 */
	class NAPAPI SequencePlayerInput : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * Constructor
		 */
		SequencePlayerInput() = default;

		/**
		 * Deconstructor
		 */
		virtual ~SequencePlayerInput() = default;
	};
}
