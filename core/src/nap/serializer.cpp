#include "serializer.h"


using namespace std;

namespace nap
{


	std::string Serializer::toString(Object& object) const
	{
		std::ostringstream ss;
		writeObject(ss, object);
		return ss.str();
	}


	Object* Deserializer::fromString(const std::string& str, Core& core, Object* parent) const
	{
		std::istringstream ss(str);
		return readObject(ss, core, parent);
	}
}
