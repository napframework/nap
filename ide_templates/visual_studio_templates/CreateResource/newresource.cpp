#include "$fileinputname$.h"

// nap::$fileinputname$ run time class definition 
RTTI_BEGIN_CLASS(nap::$fileinputname$)
	// Put additional properties here
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	$fileinputname$::~$fileinputname$()			{ }


	bool $fileinputname$::init(utility::ErrorState& errorState)
	{
		return true;
	}
}