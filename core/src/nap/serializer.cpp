#include <fstream>
#include "serializer.h"
#include "plug.h"
#include "coreattributes.h"

RTTI_DEFINE(nap::Serializer)


using namespace std;

namespace nap
{

    bool sanitizeJSONString(const std::string& str)
    {
        std::string allowed = "abcdefghijklmnopqrstuvwxyz0123456789.,{}[]:_/\"\n ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        for(unsigned long int i = 0; i < str.length(); ++i) {
            char c = str[i];
            if (allowed.find(c) == std::string::npos) {
                Logger::warn("Illegal character '%c' found while deserializing at %u", c, i);
                return false;
            }
        }
        return true;
    }


	std::string Serializer::toString(Object& object, bool writePointers) const
	{
		std::ostringstream ss;
        writeObject(ss, object, writePointers);
		return ss.str();
	}


    std::string Serializer::toString(ModuleManager& moduleManager) const {
        std::ostringstream ss;
        writeModuleInfo(ss, moduleManager);
        return ss.str();
    }


    Object* Serializer::fromString(const std::string& str, Core& core, Object* parent) const
	{
//        if (!sanitizeJSONString(str))
//            return nullptr;
		std::istringstream ss(str);
		return readObject(ss, core, parent);
	}


    Object* Serializer::load(const std::string& filename, Core& core, Object* parent) const {
        if (!fileExists(filename)) {
            Logger::warn("File does not exist: %s", getAbsolutePath(filename).c_str());
            return nullptr;
        }
        std::ifstream is(filename.c_str());
        Object* obj = readObject(is, core, parent);
        is.close();
        return obj;
    }

}
