#include "module.h"
#include "modulemanager.h"
#include <nap/operator.h>

RTTI_DEFINE(nap::Module)

namespace nap
{
	Module::Module(const std::string& name) : mName(name) {}


	bool Module::hasTypeConverter(const nap::TypeConverterBase* tc) const
	{
		for (const nap::TypeConverterBase* other : mTypeConverters)
			if (other->inType() == tc->inType() && other->outType() == tc->outType()) return true;
		return false;
	}

//	void Module::registerTypeConverter(const nap::TypeConverterBase* tc)
//	{
//		// Do not allow duplicate type converters
//	}

    const std::vector<const nap::TypeConverterBase*> Module::getTypeConverters() const { return mTypeConverters; }

}
