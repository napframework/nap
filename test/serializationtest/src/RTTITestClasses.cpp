#include "RTTITestClasses.h"

RTTI_BEGIN_CLASS(DataStruct)
	RTTI_PROPERTY("FloatProperty",				&DataStruct::mFloatProperty,				rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PointerProperty",			&DataStruct::mPointerProperty,				rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(BaseClass)
	RTTI_PROPERTY("IntProperty",				&BaseClass::mIntProperty,					rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("StringProperty",				&BaseClass::mStringProperty,				rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PointerProperty",			&BaseClass::mPointerProperty,				rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(DerivedClass)
	RTTI_PROPERTY("NestedCompound",				&DerivedClass::mNestedCompound,				rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ArrayOfInts",				&DerivedClass::mArrayOfInts,				rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ArrayOfCompounds",			&DerivedClass::mArrayOfCompounds,			rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ArrayOfPointers",			&DerivedClass::mArrayOfPointers,			rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("EmbeddedPointer",			&DerivedClass::mEmbeddedPointer,			rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("ArrayOfEmbeddedPointers",	&DerivedClass::mArrayOfEmbeddedPointers,	rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS