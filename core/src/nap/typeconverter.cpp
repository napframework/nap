#include "typeconverter.h"
#include <nap/module.h>

template<typename T>
inline std::string addresStr(T thing) {
	const void* addr = static_cast<const void*>(thing);
	std::stringstream ss;
	ss << addr;
	return ss.str();
}

namespace nap
{
	TypeConverterBase::TypeConverterBase(RTTI::TypeInfo inType, RTTI::TypeInfo outType)
		: mInType(inType), mOutType(outType)
	{
//		Module::get().registerTypeConverter(this);
	}


}