#pragma once

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	*/
	class NAPAPI KeyFrame : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		double			mTime = 0.0;
		std::string		mName = "";
	};
}
