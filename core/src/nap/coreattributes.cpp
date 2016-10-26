#include <nap/coreattributes.h>

namespace nap
{
    // Clamp implementation for float attribute
    template<>
    float NumericAttribute<float>::clampValue(const float &value, const float &min, const float &max) {
        return std::max(min, std::min(value, max));
    }


    // Clamp implementation for int attribute
    template<>
    int NumericAttribute<int>::clampValue(const int &value, const int &min, const int &max) {
        return std::max(min, std::min(value, max));
    }


    // Clamp implementation for double attribute
    template<>
    double NumericAttribute<double>::clampValue(const double &value, const double &min, const double &max) {
        return std::max(min, std::min(value, max));
    }


    // All the comparison operators for array types
    bool operator==(const FloatArray& a, const FloatArray& b)
    {
        return false;
    }


    bool operator==(const StringArray& a, const StringArray& b)
    {
        return false;
    }


    bool operator==(const IntArray& a, const IntArray& b)
    {
        return false;
    }


    bool operator==(const FloatMap& a, const FloatMap& b)
    {
        return false;
    }


    bool operator==(const IntMap& a, const IntMap& b)
    {
        return false;
    }


    bool operator==(const StringMap& a, const StringMap& b)
    {
        return false;
    }


    bool operator==(const RTTIStringMap& a, const RTTIStringMap& b)
    {
        return false;
    }


    bool operator==(const Binary& a, const Binary& b)
    {
        return false;
    }
}

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

