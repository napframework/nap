#pragma once

// Local Includes
#include "component.h"
#include "patch.h"

// External Includes
#include <rtti/rtti.h>

namespace nap
{
	//! A component containing a patch of operators with connections
	class PatchComponent : public Component
	{
		RTTI_ENABLE_DERIVED_FROM(nap::Component)
	public:
		//! Default constructor
		PatchComponent() 
		{ 
			mPatch = &addChild<Patch>("patch");
		}

        // Virtual destructor because of virtual methods!
		virtual ~PatchComponent() = default;

		//! Returns the patch within this component
		Patch& getPatch() { return *mPatch; }

		//! Returns the patch const
		const Patch& getPatch() const { return *mPatch; }

        void connect(nap::OutputPlugBase& source, nap::InputPlugBase& destination);

        void disconnnect(nap::OutputPlugBase& source, nap::InputPlugBase& destination);

	private:
		//! Child patch of this component
		Patch* mPatch = nullptr;
	};
}

RTTI_DECLARE(nap::PatchComponent)
