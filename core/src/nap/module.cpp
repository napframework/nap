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
			if (other->inType() == tc->inType() && other->outType() == tc->outType())
				return true;
		return false;
	}

	const std::vector<const nap::TypeConverterBase*> Module::getTypeConverters() const { return mTypeConverters; }

	void Module::getComponentTypes(TypeList& outTypes) const
	{
		outTypes.insert(outTypes.end(), mComponentTypes.begin(), mComponentTypes.end());
	}

	void Module::getDataTypes(TypeList& outTypes) const
	{
		outTypes.insert(outTypes.end(), mDataTypes.begin(), mDataTypes.end());
	}

	void Module::getOperatorTypes(TypeList& outTypes) const
	{
		outTypes.insert(outTypes.end(), mOperatorTypes.begin(), mOperatorTypes.end());
	}

	void Module::getServiceTypes(TypeList& outTypes)
	{
		outTypes.insert(outTypes.end(), mServiceTypes.begin(), mServiceTypes.end());
	}
}
