#include "RTTITestClasses.h"

RTTI_BEGIN_ENUM(ETestEnum)
	RTTI_ENUM_VALUE(ETestEnum::One,		"One"),
	RTTI_ENUM_VALUE(ETestEnum::Two,		"Two"),
	RTTI_ENUM_VALUE(ETestEnum::Three,	"Three"),
	RTTI_ENUM_VALUE(ETestEnum::Four,	"Four")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(DataStruct::ENestedEnum)
	RTTI_ENUM_VALUE(DataStruct::ENestedEnum::Five,		"Five"),
	RTTI_ENUM_VALUE(DataStruct::ENestedEnum::Six,		"Six"),
	RTTI_ENUM_VALUE(DataStruct::ENestedEnum::Seven,		"Seven"),
	RTTI_ENUM_VALUE(DataStruct::ENestedEnum::Eight,		"Eight")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(DataStruct)
	RTTI_PROPERTY("FloatProperty",				&DataStruct::mFloatProperty,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PointerProperty",			&DataStruct::mPointerProperty,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("NestedEnum",					&DataStruct::mNestedEnum,					nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(BaseClass)
	RTTI_PROPERTY("IntProperty",				&BaseClass::mIntProperty,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("StringProperty",				&BaseClass::mStringProperty,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PointerProperty",			&BaseClass::mPointerProperty,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("EnumProperty",				&BaseClass::mEnumProperty,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ObjectPtrProperty",			&BaseClass::mObjectPtrProperty,				nap::rtti::EPropertyMetaData::Default)	
RTTI_END_CLASS

RTTI_BEGIN_CLASS(DerivedClass)
	RTTI_PROPERTY("NestedCompound",				&DerivedClass::mNestedCompound,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ArrayOfInts",				&DerivedClass::mArrayOfInts,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ArrayOfCompounds",			&DerivedClass::mArrayOfCompounds,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ArrayOfPointers",			&DerivedClass::mArrayOfPointers,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("EmbeddedPointer",			&DerivedClass::mEmbeddedPointer,			nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("ArrayOfEmbeddedPointers",	&DerivedClass::mArrayOfEmbeddedPointers,	nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS