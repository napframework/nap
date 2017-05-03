#include "RTTITestClasses.h"

RTTI_BEGIN_CLASS(DataStruct)
	RTTI_PROPERTY("FloatProperty",		&DataStruct::mFloatProperty)
	RTTI_PROPERTY("PointerProperty",	&DataStruct::mPointerProperty)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(BaseClass)
	RTTI_PROPERTY("IntProperty",		&BaseClass::mIntProperty)
	RTTI_PROPERTY("StringProperty",		&BaseClass::mStringProperty)
	RTTI_PROPERTY("PointerProperty",	&BaseClass::mPointerProperty)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(DerivedClass)
	RTTI_PROPERTY("NestedCompound",		&DerivedClass::mNestedCompound)
	RTTI_PROPERTY("ArrayOfInts",		&DerivedClass::mArrayOfInts)
	RTTI_PROPERTY("ArrayOfCompounds",	&DerivedClass::mArrayOfCompounds)
	RTTI_PROPERTY("ArrayOfPointers",	&DerivedClass::mArrayOfPointers)
RTTI_END_CLASS