// Local Includes
#include "sdo.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SDOArgument)
	RTTI_PROPERTY("Single",			&nap::SDOArgument::mSingle,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Description",	&nap::SDOArgument::mDescription,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Index",			&nap::SDOArgument::mIndexStr,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SubIndex",		&nap::SDOArgument::mSubIndexStr,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Base",			&nap::SDOArgument::mBase,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SDO_int8)
	RTTI_PROPERTY("Value",	&nap::SDO_int8::mValue,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SDO_uint8)
	RTTI_PROPERTY("Value", &nap::SDO_uint8::mValue,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SDO_int16)
	RTTI_PROPERTY("Value", &nap::SDO_int16::mValue,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SDO_uint16)
	RTTI_PROPERTY("Value", &nap::SDO_uint16::mValue, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SDO_int32)
	RTTI_PROPERTY("Value", &nap::SDO_int32::mValue, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SDO_uint32)
	RTTI_PROPERTY("Value", &nap::SDO_uint32::mValue, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SDO_int64)
	RTTI_PROPERTY("Value", &nap::SDO_int64::mValue, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SDO_uint64)
	RTTI_PROPERTY("Value", &nap::SDO_uint64::mValue, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SDO_float)
	RTTI_PROPERTY("Value", &nap::SDO_float::mValue, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SDO_double)
	RTTI_PROPERTY("Value", &nap::SDO_double::mValue, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SDOGroup)
	RTTI_PROPERTY("Objects", &nap::SDOGroup::mObjects, nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS


namespace nap
{
	SDOArgument::SDOArgument(uint16 index, uint16 subIndex, bool single) :
		mIndex(index), mSubIndex(subIndex), mSingle(single)
	{ }


	SDOArgument::SDOArgument(uint16 index, uint16 subIndex, const std::string& name, bool single) :
		mIndex(index), mSubIndex(subIndex), mDescription(name), mSingle(single)
	{ }


	bool SDOArgument::init(utility::ErrorState& errorState)
	{
		mIndex = static_cast<uint16>(std::stoul(mIndexStr, nullptr, mBase));
		mSubIndex = static_cast<uint16>(std::stoul(mSubIndexStr, nullptr, mBase));
		return true;
	}
}