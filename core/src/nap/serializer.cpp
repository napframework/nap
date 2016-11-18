#include "serializer.h"


using namespace std;

namespace nap
{


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


    Object* Deserializer::fromString(const std::string& str, Core& core, Object* parent) const
	{

        std::string allowed = "abcdefghijklmnopqrstuvwxyz0123456789.,{}[]:\"";
        for(unsigned long int i = 0; i < str.length(); ++i) {
            if (allowed.find(str[i]) == std::string::npos) {
                Logger::warn("Illegal characters found while deserializing.");
                Logger::debug(str);
                return nullptr;
            }
        }

		std::istringstream ss(str);
		return readObject(ss, core, parent);
	}
}
