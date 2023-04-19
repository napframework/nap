#include "$fileinputname$.h"

// External Includes
#include <entity.h>

// nap::$fileinputname$ run time class definition 
RTTI_BEGIN_CLASS(nap::$fileinputname$)
	// Put additional properties here
RTTI_END_CLASS

// nap::$fileinputname$Instance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::$fileinputname$Instance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void $fileinputname$::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool $fileinputname$Instance::init(utility::ErrorState& errorState)
	{
		return true;
	}


	void $fileinputname$Instance::update(double deltaTime)
	{

	}
}