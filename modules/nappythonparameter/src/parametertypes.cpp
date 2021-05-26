#include "parametertypes.h"

DEFINE_SIMPLE_PARAMETER(nap::ParameterString)

RTTI_BEGIN_CLASS(nap::ParameterOptionList)
    RTTI_PROPERTY("Value",		&nap::ParameterOptionList::mValue,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Options",	&nap::ParameterOptionList::mOptions,	nap::rtti::EPropertyMetaData::Default)
    RTTI_FUNCTION("setValue",	static_cast<void (nap::ParameterOptionList::*)(int)>(&nap::ParameterOptionList::setValue))
	RTTI_FUNCTION("getValue", &nap::ParameterOptionList::getValue)
    RTTI_FUNCTION("setOption", &nap::ParameterOptionList::setOption)
	RTTI_FUNCTION("getOptionName", &nap::ParameterOptionList::getOptionName)
RTTI_END_CLASS


namespace nap
{

    void ParameterOptionList::setValue(const Parameter& value)
    {
        const auto* other = rtti_cast<const ParameterOptionList>(&value);
        assert(other != nullptr);
        setValue(other->getValue());
    }


    bool ParameterOptionList::setOption(const std::string& optionName)
    {
        for (auto i = 0; i < mOptions.size(); ++i)
            if (mOptions[i] == optionName)
            {
                mValue = i;
                valueChanged(mValue);
                return true;
            }

        return false;
    }


    void ParameterOptionList::setOptions(const std::vector<std::string>& options)
    {
        assert(!options.empty());
        mOptions = options;
        mValue = 0;
    }


    void ParameterOptionList::setValue(int index)
    {
        assert(index < mOptions.size());
        mValue = index;
        valueChanged(mValue);
    }


}
