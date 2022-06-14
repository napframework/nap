#include "group.h"

// nap::Resource run time class definition 
RTTI_BEGIN_CLASS(nap::Group)
	RTTI_PROPERTY("Group", &nap::Group::mResources, nap::rtti::EPropertyMetaData::Embedded | nap::rtti::EPropertyMetaData::ReadOnly)
RTTI_END_CLASS

namespace nap
{ }
