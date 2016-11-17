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
		std::istringstream ss(str);
		return readObject(ss, core, parent);
	}
}
