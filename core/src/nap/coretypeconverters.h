#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace nap {
    
    bool convertStringToFloat(const std::string& inValue, float& outValue);
    bool convertFloatToString(const float& inValue, std::string& outString);
    
    bool convertStringToInt(const std::string& inValue, int& outValue);
    bool convertIntToString(const int& inValue, std::string& outString);
    
    bool convertStringToDouble(const std::string& inValue, double outValue);
    bool convertDoubleToString(const double& inValue, std::string& outString);
    
    bool convertStringToBool(const std::string& inString, bool& outValue);
    bool convertBoolToString(const bool& inValue, std::string& inString);
    
    bool convertStringToFloatArray(const std::string& inString, std::vector<float>& outValue);
    bool convertFloatArrayToString(const std::vector<float>& inValue, std::string& ioString);
    bool convertBinaryToFloatArray(const std::vector<char>& inValue, std::vector<float>& outValue);
    bool convertFloatArrayToBinary(const std::vector<float>& inValue, std::vector<char>& outValue);    
    
    bool convertStringToStringArray(const std::string& inString, std::vector<std::string>& outValue);
    bool convertStringArrayToString(const std::vector<std::string>& inValue, std::string& outString);
    
    bool convertIntArrayToString(const std::vector<int>& inValue, std::string& outString);
    bool convertStringToIntArray(const std::string& inString, std::vector<int>& outValue);
    
    bool convertStringToFloatMap(const std::string& inString, std::unordered_map<std::string, float>& outValue);
    bool convertFloatMapToString(const std::unordered_map<std::string, float>& inValue, std::string& ioString);
    
    bool convertStringToIntMap(const std::string& inString, std::unordered_map<std::string, int>& outValue);
    bool convertIntMapToString(const std::unordered_map<std::string, int>& inValue, std::string& ioString);

//    bool convertFloatToInt(const float& inValue, int& outValue) { outValue = inValue; return true; }
//    bool convertIntToFloat(const int& inValue, float& outValue) { outValue = inValue; return true; }
}


