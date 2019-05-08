#include "testclasses.h"
#include <entity.h>

RTTI_BEGIN_ENUM(TestEnum)
		RTTI_ENUM_VALUE(TestEnum::Undefined, "Undefined"),
		RTTI_ENUM_VALUE(TestEnum::Alpha, "Alpha"),
		RTTI_ENUM_VALUE(TestEnum::Beta, "Beta"),
		RTTI_ENUM_VALUE(TestEnum::Gamma, "Gamma")
RTTI_END_ENUM


RTTI_BEGIN_CLASS(TestPropertiesStruct)
		RTTI_PROPERTY("String",  &TestPropertiesStruct::mString,  nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Strings", &TestPropertiesStruct::mStrings, nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY_FILELINK("Filename", &TestPropertiesStruct::mFilename, nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Font)
		RTTI_PROPERTY("Int",     &TestPropertiesStruct::mInt,     nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints",    &TestPropertiesStruct::mInts,    nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints2D",  &TestPropertiesStruct::mInts2D,  nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Float",   &TestPropertiesStruct::mFloat,   nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Floats",  &TestPropertiesStruct::mFloats,  nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Enum",    &TestPropertiesStruct::mEnum,    nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Enums",   &TestPropertiesStruct::mEnums,   nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


RTTI_BEGIN_CLASS(TestResource)
		RTTI_PROPERTY("String",   &TestResource::mString,  nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Strings",  &TestResource::mStrings, nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Int",      &TestResource::mInt,     nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints",     &TestResource::mInts,    nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints2D",   &TestResource::mInts2D,  nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Float",    &TestResource::mFloat,   nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Floats",   &TestResource::mFloats,  nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Struct",   &TestResource::mStruct,  nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Structs",  &TestResource::mStructs, nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Enum",     &TestResource::mEnum,    nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Enums",    &TestResource::mEnums,   nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


RTTI_BEGIN_CLASS(TestResourceB)
		RTTI_PROPERTY("ResPointer",     &TestResourceB::mResPointer,    nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("ResPointers",    &TestResourceB::mResPointers,   nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("EmbedPointer",   &TestResourceB::mEmbedPointer,  nap::rtti::EPropertyMetaData::Embedded)
		RTTI_PROPERTY("EmbedPointers",  &TestResourceB::mEmbedPointers, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(TestComponentInstance)
		RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(TestComponent)
		RTTI_PROPERTY("String",    &TestComponent::mString,    nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Strings",   &TestComponent::mStrings,   nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Int",       &TestComponent::mInt,       nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints",      &TestComponent::mInts,      nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints2D",    &TestComponent::mInts2D,    nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Float",     &TestComponent::mFloat,     nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Floats",    &TestComponent::mFloats,    nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Struct",    &TestComponent::mStruct,    nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Structs",   &TestComponent::mStructs,   nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Enum",      &TestComponent::mEnum,      nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Enums",     &TestComponent::mEnums,     nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Resource",  &TestComponent::mResource,  nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Resources", &TestComponent::mResources, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


RTTI_BEGIN_CLASS(TestComponentB)
		RTTI_PROPERTY("CompPointer",  &TestComponentB::mCompPointer,  nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("CompPointers", &TestComponentB::mCompPointers, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS