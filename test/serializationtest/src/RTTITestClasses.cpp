#include "RTTITestClasses.h"

RTTI_BEGIN_CLASS(DataStruct)
	RTTI_PROPERTY("FloatProperty",				&DataStruct::mFloatProperty,				RTTI::EPropertyMetaData::Default)
	RTTI_PROPERTY("PointerProperty",			&DataStruct::mPointerProperty,				RTTI::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(BaseClass)
	RTTI_PROPERTY("IntProperty",				&BaseClass::mIntProperty,					RTTI::EPropertyMetaData::Default)
	RTTI_PROPERTY("StringProperty",				&BaseClass::mStringProperty,				RTTI::EPropertyMetaData::Default)
	RTTI_PROPERTY("PointerProperty",			&BaseClass::mPointerProperty,				RTTI::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(DerivedClass)
	RTTI_PROPERTY("NestedCompound",				&DerivedClass::mNestedCompound,				RTTI::EPropertyMetaData::Default)
	RTTI_PROPERTY("ArrayOfInts",				&DerivedClass::mArrayOfInts,				RTTI::EPropertyMetaData::Default)
	RTTI_PROPERTY("ArrayOfCompounds",			&DerivedClass::mArrayOfCompounds,			RTTI::EPropertyMetaData::Default)
	RTTI_PROPERTY("ArrayOfPointers",			&DerivedClass::mArrayOfPointers,			RTTI::EPropertyMetaData::Default)
	RTTI_PROPERTY("EmbeddedPointer",			&DerivedClass::mEmbeddedPointer,			RTTI::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("ArrayOfEmbeddedPointers",	&DerivedClass::mArrayOfEmbeddedPointers,	RTTI::EPropertyMetaData::Embedded)
RTTI_END_CLASS