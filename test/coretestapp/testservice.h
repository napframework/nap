#pragma once

#include <nap/service.h>
#include <rtti/rtti.h>

namespace nap
{
	class TestService : public Service
	{
        static void sRegisterTypes(nap::Core& inCore, nap::Service& service) { }
        
		// Enable RTTI
		RTTI_ENABLE()

		// Register Types
		NAP_DECLARE_SERVICE()

	public:
		TestService()	{ }
	};
}

RTTI_DECLARE_BASE(nap::TestService)


