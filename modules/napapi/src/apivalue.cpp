#include "apivalue.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIBaseValue)
	RTTI_PROPERTY("Name", &nap::APIBaseValue::mName, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIFloat)
	RTTI_CONSTRUCTOR(const std::string&, const float&)
	RTTI_PROPERTY("Value", &nap::APIFloat::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIBool)
	RTTI_CONSTRUCTOR(const std::string&, const bool&)
	RTTI_PROPERTY("Value", &nap::APIBool::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIInt)
	RTTI_CONSTRUCTOR(const std::string&, const int&)
	RTTI_PROPERTY("Value", &nap::APIInt::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIChar)
	RTTI_CONSTRUCTOR(const std::string&, const char&)
	RTTI_PROPERTY("Value", &nap::APIChar::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIString)
	RTTI_CONSTRUCTOR(const std::string&, const std::string&)
	RTTI_PROPERTY("Value", &nap::APIString::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIDouble)
	RTTI_CONSTRUCTOR(const std::string&, const double&)
	RTTI_PROPERTY("Value", &nap::APIDouble::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APILong)
	RTTI_CONSTRUCTOR(const std::string&, const long&)
	RTTI_PROPERTY("Value", &nap::APILong::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIByte)
	RTTI_CONSTRUCTOR(const std::string&, const uint8_t&)
	RTTI_PROPERTY("Value", &nap::APIByte::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIFloatArray)
	RTTI_CONSTRUCTOR(const std::string&, const std::vector<float>&)
	RTTI_PROPERTY("Value", &nap::APIFloatArray::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIIntArray)
	RTTI_CONSTRUCTOR(const std::string&, const std::vector<int>&)
	RTTI_PROPERTY("Value", &nap::APIIntArray::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APICharArray)
	RTTI_CONSTRUCTOR(const std::string&, const std::vector<char>&)
	RTTI_PROPERTY("Value", &nap::APICharArray::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIStringArray)
	RTTI_CONSTRUCTOR(const std::string&, const std::vector<std::string>&)
	RTTI_PROPERTY("Value", &nap::APIStringArray::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::APIByteArray)
	RTTI_CONSTRUCTOR(const std::string&, const std::vector<uint8_t>&)
	RTTI_PROPERTY("Value", &nap::APIByteArray::mValue, nap::rtti::EPropertyMetaData::Default)
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