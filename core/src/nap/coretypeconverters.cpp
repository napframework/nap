// Local
#include "coretypeconverters.h"
#include "stringutils.h"
#include "logger.h"

// External
#include <cstring>
#include <sstream>
#include <cassert>

using namespace std;

namespace nap {
    
    bool convertStringToFloat(const string& inValue, float& outValue)
    {
        outValue = static_cast<float>(atof(inValue.c_str()));
        return true;
    }
    
    
    
    bool convertFloatToString(const float& inValue, string& outString)
    {
        ostringstream ss;
        ss << inValue;
        outString = ss.str();
        return true;
    }
    
    
    
    bool convertStringToInt(const string& inValue, int& outValue)
    {
        outValue = atoi(inValue.c_str());
        return true;
    }
    
    
    
    bool convertIntToString(const int& inValue, string& outString)
    {
        ostringstream ss;
        ss << inValue;
        outString = ss.str();
        return true;
    }


    
    bool convertStringToDouble(const string& inValue, double outValue)
    {
        outValue = atof(inValue.c_str());
        return true;
    }
    
    
    
    bool convertDoubleToString(const double& inValue, string& outString)
    {
        std::ostringstream ss;
        ss << inValue;
        outString = ss.str();
        return true;
    }
    
    
    // Typedefs true / false
    const static std::string sTString("true");
    const static std::string sFString("false");

    
    bool convertStringToBool(const string& inString, bool& outValue)
    {
        outValue = (inString == sTString);
        return true;
    }
    
    
    bool convertBoolToString(const bool& inValue, string& inString)
    {
        inString = inValue ? sTString : sFString;
        return true;
    }
    
    
    
    bool convertStringToFloatArray(const string& inString, vector<float>& outValue)
    {
        outValue.clear();
        std::vector<std::string> out_string;
        gSplitString(inString, ',', out_string);
        for (auto& value : out_string) {
            outValue.emplace_back((float) atof(value.c_str()));
        }
        return true;
    }
    
    
    bool convertFloatArrayToString(const vector<float>& inValue, string& outString)
    {
        std::ostringstream ss;
        for (auto it = inValue.begin(); it != inValue.end(); it++) {
            ss << (*it);
            if (std::next(it) != inValue.end()) {
                ss << ',';
            }
        }
        outString = ss.str();
        return true;
    }
    
    
    bool convertStringToStringArray(const std::string& inString, vector<string>& outValue)
    {
        outValue.clear();
        std::vector<std::string> out_string;
        gSplitString(inString, ',', out_string);
        for (auto& pair : out_string) {
            outValue.emplace_back(pair);
        }
        return true;
    }
    
    
    bool convertStringArrayToString(const vector<string>& inValue, std::string& outString)
    {
        std::ostringstream ss;
        for (auto it = inValue.begin(); it != inValue.end(); it++) 
		{
			if (gContains(*it, ","))
			{
				nap::Logger::warn("Invalid serialization string character: %s", it->c_str());
				continue;
			}

            ss << (*it);
            if (std::next(it) != inValue.end()) {
                ss << ',';
            }
        }
        outString = ss.str();
        return true;
    }
    
    
    bool convertIntArrayToString(const vector<int>& inValue, std::string& outString)
    {
        std::ostringstream ss;
        for (auto it = inValue.begin(); it != inValue.end(); it++) {
            ss << (*it);
            if (std::next(it) != inValue.end()) {
                ss << ',';
            }
        }
        outString = ss.str();
        return true;
    }
    
    
    bool convertStringToIntArray(const std::string& inString, vector<int>& outValue)
    {
        outValue.clear();
        std::vector<std::string> out_string;
        gSplitString(inString, ',', out_string);
        for (auto& value : out_string) {
            outValue.emplace_back(atoi(value.c_str()));
        }
        return true;
    }
    
    
    bool convertStringToFloatMap(const std::string& inString, unordered_map<string, float>& outValue)
    {
        outValue.clear();
        std::vector<std::string> out_string;
        gSplitString(inString, ',', out_string);
        for (auto& pair : out_string) {
            std::vector<std::string> keyvalue;
            gSplitString(pair, ':', keyvalue);
            assert(keyvalue.size() == 2);
            float v = (float)atof(keyvalue[1].c_str());
            outValue.emplace(std::make_pair(keyvalue[0], v));
        }
        return true;
    }
    
    
    bool convertFloatMapToString(const unordered_map<string, float>& inValue, std::string& ioString)
    {
        std::ostringstream ss;
        for (auto it = inValue.begin(); it != inValue.end(); it++) {
            ss << (*it).first;
            ss << ':';
            ss << (*it).second;
            if (std::next(it) != inValue.end()) {
                ss << ',';
            }
        }
        ioString = ss.str();
        return true;
    }
    
    
    bool convertStringToIntMap(const std::string& inString, unordered_map<string, int>& outValue)
    {
        outValue.clear();
        std::vector<std::string> out_string;
        gSplitString(inString, ',', out_string);
        for (auto& pair : out_string) {
            std::vector<std::string> keyvalue;
            gSplitString(pair, ':', keyvalue);
            assert(keyvalue.size() == 2);
            int v = atoi(keyvalue[1].c_str());
            outValue[keyvalue[0]] = v;
        }
        return true;
    }
    
    
    bool convertIntMapToString(const unordered_map<string, int>& inValue, string& ioString)
    {
        std::ostringstream ss;
        for (auto it = inValue.begin(); it != inValue.end(); it++) {
            ss << (*it).first;
            ss << ':';
            ss << (*it).second;
            if (std::next(it) != inValue.end()) {
                ss << ',';
            }
        }
        ioString = ss.str();
        return true;
    }
    
    
    bool convertBinaryToFloatArray(const vector<char>& inValue, vector<float>& outValue)
    {
        outValue.resize(inValue.size() / sizeof(float));
        memcpy(outValue.data(), inValue.data(), outValue.size());
        return true;
    }
    
    
    bool convertFloatArrayToBinary(const vector<float>& inValue, vector<char>& outValue)
    {
        outValue.resize(inValue.size() * sizeof(float));
        memcpy(outValue.data(), inValue.data(), outValue.size());
        return true;
    }
    
    
    
    

}