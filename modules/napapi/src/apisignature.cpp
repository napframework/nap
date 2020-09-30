#include "apisignature.h"

// nap::apisignature run time class definition 
RTTI_BEGIN_CLASS(nap::APISignature)
	RTTI_PROPERTY("Arguments",  &nap::APISignature::mArguments, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	APISignature::~APISignature()			{ }
}