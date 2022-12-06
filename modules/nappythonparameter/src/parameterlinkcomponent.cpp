#include "parameterlinkcomponent.h"
#include <parameternumeric.h>
#include <parametertypes.h>
#include <entity.h>


// Python parameter wrapper definitions
namespace nap
{
	std::map<rtti::TypeInfo, ParameterLinkComponent::ParameterWrapperCreationFunction> ParameterLinkComponent::sParameterWrapperFactory;

	using FloatPythonParameterWrapper = PythonParameterWrapper<ParameterFloat>;
	using IntPythonParameterWrapper = PythonParameterWrapper<ParameterInt>;
	using OptionListPythonParameterWrapper = PythonParameterWrapper<ParameterOptionList>;
	using BoolPythonParameterWrapper = PythonParameterWrapper<ParameterBool>;
	using Vec2PythonParameterWrapper = PythonParameterWrapper<ParameterVec2>;
	using Vec3PythonParameterWrapper = PythonParameterWrapper<ParameterVec3>;
}

DEFINE_PYTHON_PARAMETER_WRAPPER(nap::BoolPythonParameterWrapper)
DEFINE_PYTHON_PARAMETER_WRAPPER(nap::FloatPythonParameterWrapper)
DEFINE_PYTHON_PARAMETER_WRAPPER(nap::IntPythonParameterWrapper)
DEFINE_PYTHON_PARAMETER_WRAPPER(nap::OptionListPythonParameterWrapper)
DEFINE_PYTHON_PARAMETER_WRAPPER(nap::Vec2PythonParameterWrapper)
DEFINE_PYTHON_PARAMETER_WRAPPER(nap::Vec3PythonParameterWrapper)

RTTI_BEGIN_CLASS(nap::ParameterLinkComponent)
	RTTI_PROPERTY("ParameterGroup", &nap::ParameterLinkComponent::mParameterGroup, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterLinkComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
	RTTI_FUNCTION("findParameter", &nap::ParameterLinkComponentInstance::getParameterWrapper)
RTTI_END_CLASS

namespace nap
{

	bool ParameterLinkComponentInstance::init(utility::ErrorState& errorState)
	{
		mParameterGroup = getComponent<ParameterLinkComponent>()->mParameterGroup;
		return true;
	}


	rtti::Object* ParameterLinkComponentInstance::getParameterWrapper(const std::string& mID)
	{
		// first find the parameter in the group
		auto parameter = mParameterGroup->findObject(mID);
		if (parameter == nullptr)
			return nullptr;

		// is there already a wrapper created?
		{
			auto it = mPythonParameterWrappers.find(parameter.get());
			if (it != mPythonParameterWrappers.end())
			{
				return it->second.get();
			}
		}

		// is there a wrapper type registered
		{
			auto& wrapperFactory = ParameterLinkComponent::sParameterWrapperFactory;
			auto it = wrapperFactory.find(parameter->get_type());
			if (it != wrapperFactory.end())
			{
				auto wrapper = it->second(parameter.get());
				auto wrapperPtr = wrapper.get();
				mPythonParameterWrappers[parameter.get()] = std::move(wrapper);
				return wrapperPtr;
			}
		}

		return nullptr;
	}

}
