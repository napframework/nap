#include "compositioncontainer.h"

// nap::compositioncontainer run time class definition 
RTTI_BEGIN_CLASS(nap::CompositionContainer)
	RTTI_PROPERTY("Compositions", &nap::CompositionContainer::mCompositions, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	CompositionContainer::~CompositionContainer()			{ }


	bool CompositionContainer::init(utility::ErrorState& errorState)
	{
		return true;
	}
}