#pragma once

#include <nap/coreoperators.h>
#include <nap/coretypeconverters.h>
#include <nap/link.h>
#include <nap/module.h>
#include <nap/patchcomponent.h>
#include <rtti/rtti.h>

namespace nap
{
	// Typedefs
	using FloatArray = std::vector<float>;
	using StringArray = std::vector<std::string>;
	using IntArray = std::vector<int>;
	using FloatMap = std::unordered_map<std::string, float>;
	using IntMap = std::unordered_map<std::string, int>;
	using StringMap = std::unordered_map<std::string, std::string>;
	using RTTIStringMap = std::unordered_map<RTTI::TypeInfo, std::string>;
	using Binary = std::vector<char>;
}

// RTTI Implementation default values
RTTI_DECLARE_DATA(float)
RTTI_DECLARE_DATA(int)
RTTI_DECLARE_DATA(std::string)
RTTI_DECLARE_DATA(double)
RTTI_DECLARE_DATA(bool)

// TODO: When declaring TypeInfos, automatically enable array types.
RTTI_DECLARE_DATA(nap::FloatArray)
RTTI_DECLARE_DATA(nap::StringArray)
RTTI_DECLARE_DATA(nap::IntArray)
RTTI_DECLARE_DATA(nap::IntMap)
RTTI_DECLARE_DATA(nap::FloatMap)
RTTI_DECLARE_DATA(nap::StringMap)
RTTI_DECLARE_DATA(nap::Binary)
RTTI_DECLARE_DATA(nap::RTTIStringMap)

class ModuleNapCore : public nap::Module
{
	RTTI_ENABLE_DERIVED_FROM(nap::Module)
public:
	ModuleNapCore();
};
RTTI_DECLARE(ModuleNapCore)
