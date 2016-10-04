#include <nofattributewrapper.h>
#include <nap/coremodule.h>

/**
@brief Adds an attribute object as a set of parameters to the managed group
**/
void OFAttributeWrapper::addObject(nap::AttributeObject& object)
{
	for (auto& attribute : object.getAttributes())
	{
		addAttribute(*attribute);
	}
}


/**
@brief Add a single attribute as a parameter to the managed group
**/
void OFAttributeWrapper::addAttribute(nap::AttributeBase& attribute)
{
	if (attribute.getTypeInfo().isKindOf(RTTI_OF(nap::NumericAttribute<float>)))
	{
		nap::NumericAttribute<float>& attr = static_cast<nap::NumericAttribute<float>&>(attribute);
		mGroup.add(ofParameter<float>(attr.getName(), attr.getValue(), 0.0f, 1.0f));
	}
}

