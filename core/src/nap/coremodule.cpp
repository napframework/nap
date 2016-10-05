#include <nap/coremodule.h>
#include <nap/eventdispatcher.h>
#include <nap/configure.h>

//////////////////////////////////////////////////////////////////////////
// Numeric Attribute template specializations
//////////////////////////////////////////////////////////////////////////

template <>
float nap::NumericAttribute<float>::clampValue(const float& value, const float& min, const float &max)
{
	return std::max(min, std::min(value, max));
}

template <>
int nap::NumericAttribute<int>::clampValue(const int& value, const int& min, const int&max)
{
	return std::max(min, std::min(value, max));
}

template <>
double nap::NumericAttribute<double>::clampValue(const double& value, const double& min, const double &max)
{
	return std::max(min, std::min(value, max));
}


//////////////////////////////////////////////////////////////////////////
// Default comparison operators
//////////////////////////////////////////////////////////////////////////

bool nap::operator==(const FloatArray& a, const FloatArray& b)
{
	return false;
}

bool nap::operator==(const StringArray& a, const StringArray& b)
{
	return false;
}

bool nap::operator==(const IntArray& a, const IntArray& b)
{
	return false;
}

bool nap::operator==(const FloatMap& a, const FloatMap& b)
{
	return false;
}

bool nap::operator==(const IntMap& a, const IntMap& b)
{
	return false;
}

bool nap::operator==(const StringMap& a, const StringMap& b)
{
	return false;
}

bool nap::operator==(const RTTIStringMap& a, const RTTIStringMap& b)
{
	return false;
}

bool nap::operator==(const Binary& a, const Binary& b)
{
	return false;
}

//////////////////////////////////////////////////////////////////////////
// RTTI
//////////////////////////////////////////////////////////////////////////

RTTI_DEFINE(ModuleNapCore)

// Attribute numeric data types
RTTI_DEFINE_NUMERIC_DATA(float)
RTTI_DEFINE_NUMERIC_DATA(int)
RTTI_DEFINE_NUMERIC_DATA(double)

// Attribute data types
RTTI_DEFINE_DATA(std::string)
RTTI_DEFINE_DATA(bool)

// Attribute Array types
RTTI_DEFINE_DATA(nap::FloatArray)
RTTI_DEFINE_DATA(nap::StringArray)
RTTI_DEFINE_DATA(nap::IntArray)
RTTI_DEFINE_DATA(nap::IntMap)
RTTI_DEFINE_DATA(nap::FloatMap)
RTTI_DEFINE_DATA(nap::StringMap)
RTTI_DEFINE_DATA(nap::Binary)


ModuleNapCore::ModuleNapCore() : nap::Module("NapCore") {
    // Types
    NAP_REGISTER_DATATYPE(float)
    NAP_REGISTER_DATATYPE(int)
    NAP_REGISTER_DATATYPE(std::string)
	NAP_REGISTER_DATATYPE(nap::IntArray)
	NAP_REGISTER_DATATYPE(nap::FloatArray)
	NAP_REGISTER_DATATYPE(nap::StringArray)
	NAP_REGISTER_DATATYPE(nap::FloatMap)
	NAP_REGISTER_DATATYPE(nap::IntMap)
	NAP_REGISTER_DATATYPE(nap::StringMap)
	NAP_REGISTER_DATATYPE(nap::DispatchMethod)

    // Components
    NAP_REGISTER_COMPONENT(nap::PatchComponent)

    // Operators
    NAP_REGISTER_OPERATOR(nap::AddFloatOperator)
    NAP_REGISTER_OPERATOR(nap::AddFloatOperator)
    NAP_REGISTER_OPERATOR(nap::MultFloatOperator)
    NAP_REGISTER_OPERATOR(nap::SimpleTriggerOperator)

    // TypeConverters
    NAP_REGISTER_TYPECONVERTER(nap::convertStringToFloat)
    NAP_REGISTER_TYPECONVERTER(nap::convertFloatToString)
    NAP_REGISTER_TYPECONVERTER(nap::convertStringToInt)
    NAP_REGISTER_TYPECONVERTER(nap::convertIntToString)
    NAP_REGISTER_TYPECONVERTER(nap::convertStringToIntArray)
    NAP_REGISTER_TYPECONVERTER(nap::convertIntArrayToString)
    NAP_REGISTER_TYPECONVERTER(nap::convertStringToFloatArray)
    NAP_REGISTER_TYPECONVERTER(nap::convertFloatArrayToString)
    NAP_REGISTER_TYPECONVERTER(nap::convertBinaryToFloatArray)
    NAP_REGISTER_TYPECONVERTER(nap::convertFloatArrayToBinary)
    NAP_REGISTER_TYPECONVERTER(nap::convertStringToBool)
    NAP_REGISTER_TYPECONVERTER(nap::convertBoolToString)
    NAP_REGISTER_TYPECONVERTER(nap::convertStringToIntMap)
    NAP_REGISTER_TYPECONVERTER(nap::convertIntMapToString)
    NAP_REGISTER_TYPECONVERTER(nap::convertStringToFloatMap)
    NAP_REGISTER_TYPECONVERTER(nap::convertFloatMapToString)
	NAP_REGISTER_TYPECONVERTER(nap::convert_string_to_dispatchmethod)
	NAP_REGISTER_TYPECONVERTER(nap::convert_dispatchmethod_to_string)
}
