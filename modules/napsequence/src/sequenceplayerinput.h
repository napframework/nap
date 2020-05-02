#pragma once

// nap includes
#include <nap/resource.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	class NAPAPI SequencePlayerInput : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		SequencePlayerInput() = default;
		virtual ~SequencePlayerInput() = default;
	};
}
