/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "apivalue.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIBaseValue)
	RTTI_PROPERTY("Name", &nap::APIBaseValue::mName, nap::rtti::EPropertyMetaData::Required, "Value name")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIFloat, "Float value that is sent or received")
	RTTI_CONSTRUCTOR(const std::string&, const float&)
	RTTI_PROPERTY("Value", &nap::APIFloat::mValue, nap::rtti::EPropertyMetaData::Default, "Float value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIBool, "Bool value that is sent or received")
	RTTI_CONSTRUCTOR(const std::string&, const bool&)
	RTTI_PROPERTY("Value", &nap::APIBool::mValue, nap::rtti::EPropertyMetaData::Default, "Bool value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIInt, "Integer value that is sent or received")
	RTTI_CONSTRUCTOR(const std::string&, const int&)
	RTTI_PROPERTY("Value", &nap::APIInt::mValue, nap::rtti::EPropertyMetaData::Default, "Integer value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIChar, "Char value that is sent or received")
	RTTI_CONSTRUCTOR(const std::string&, const char&)
	RTTI_PROPERTY("Value", &nap::APIChar::mValue, nap::rtti::EPropertyMetaData::Default, "Char value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIString, "String value that is sent or received")
	RTTI_CONSTRUCTOR(const std::string&, const std::string&)
	RTTI_PROPERTY("Value", &nap::APIString::mValue, nap::rtti::EPropertyMetaData::Default, "String value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIDouble, "Double value that is sent or received")
	RTTI_CONSTRUCTOR(const std::string&, const double&)
	RTTI_PROPERTY("Value", &nap::APIDouble::mValue, nap::rtti::EPropertyMetaData::Default, "Double value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APILong, "Long value that is sent or received")
	RTTI_CONSTRUCTOR(const std::string&, const long&)
	RTTI_PROPERTY("Value", &nap::APILong::mValue, nap::rtti::EPropertyMetaData::Default, "Long value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIByte, "Byte value that is sent or received")
	RTTI_CONSTRUCTOR(const std::string&, const uint8_t&)
	RTTI_PROPERTY("Value", &nap::APIByte::mValue, nap::rtti::EPropertyMetaData::Default, "Byte value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIFloatArray, "Float array that is sent or received")
	RTTI_CONSTRUCTOR(const std::string&, const std::vector<float>&)
	RTTI_PROPERTY("Value", &nap::APIFloatArray::mValue, nap::rtti::EPropertyMetaData::Default, "Float array value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIIntArray, "Integer array that is sent or received")
	RTTI_CONSTRUCTOR(const std::string&, const std::vector<int>&)
	RTTI_PROPERTY("Value", &nap::APIIntArray::mValue, nap::rtti::EPropertyMetaData::Default, "Integer array value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APICharArray, "Char array that is sent or received")
	RTTI_CONSTRUCTOR(const std::string&, const std::vector<char>&)
	RTTI_PROPERTY("Value", &nap::APICharArray::mValue, nap::rtti::EPropertyMetaData::Default, "Char array value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIStringArray, "String array that is sent or received")
	RTTI_CONSTRUCTOR(const std::string&, const std::vector<std::string>&)
	RTTI_PROPERTY("Value", &nap::APIStringArray::mValue, nap::rtti::EPropertyMetaData::Default, "String array value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIByteArray, "Byte array that is sent or received")
	RTTI_CONSTRUCTOR(const std::string&, const std::vector<uint8_t>&)
	RTTI_PROPERTY("Value", &nap::APIByteArray::mValue, nap::rtti::EPropertyMetaData::Default, "Byte array value")
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Template specializations, necessary to correctly initialize initial value of an API value.
	//////////////////////////////////////////////////////////////////////////

	template<>
	nap::APIValue<float>::APIValue() : APIBaseValue(RTTI_OF(float)), mValue(0.0f) { }

	template<>
	nap::APIValue<double>::APIValue() : APIBaseValue(RTTI_OF(double)), mValue(0.0) { }

	template<>
	nap::APIValue<int>::APIValue() : APIBaseValue(RTTI_OF(int)), mValue(0) { }

	template<>
	nap::APIValue<char>::APIValue() : APIBaseValue(RTTI_OF(char)), mValue(0) { }

	template<>
	nap::APIValue<uint8_t>::APIValue() : APIBaseValue(RTTI_OF(uint8_t)), mValue(0) { }

	template<>
	nap::APIValue<int64_t>::APIValue() : APIBaseValue(RTTI_OF(int64_t)), mValue(0) { }

	template<>
	nap::APIValue<bool>::APIValue() : APIBaseValue(RTTI_OF(bool)), mValue(false) { }

	template<>
	nap::APIValue<std::string>::APIValue() : APIBaseValue(RTTI_OF(std::string)), mValue("") { }

	template<>
	nap::APIValue<std::vector<float>>::APIValue() : APIBaseValue(RTTI_OF(std::vector<float>)) { }

	template<>
	nap::APIValue<std::vector<int>>::APIValue() : APIBaseValue(RTTI_OF(std::vector<int>)) { }

	template<>
	nap::APIValue<std::vector<char>>::APIValue() : APIBaseValue(RTTI_OF(std::vector<char>)) { }

	template<>
	nap::APIValue<std::vector<uint8_t>>::APIValue() : APIBaseValue(RTTI_OF(std::vector<uint8_t>)) { }

	template<>
	nap::APIValue<std::vector<std::string>>::APIValue() : APIBaseValue(RTTI_OF(std::vector<std::string>)) { }

	template<>
	nap::APIValue<std::vector<double>>::APIValue() : APIBaseValue(RTTI_OF(std::vector<double>)) { }
}
