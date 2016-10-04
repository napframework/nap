#include <nofattributewrapper.h>
#include <nap/coremodule.h>

OFParameterMap OFAttributeWrapper::sCreationMap;

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
	OFAbstractParamAttrLink* link = sCreateLinkedParameter(attribute);
	if (link == nullptr)
		return;

	// Add the parameter to the group
	mGroup.add(*link->getParameter());

	// Add to unique pointers for internal management
	std::unique_ptr<OFAbstractParamAttrLink> ptr(link);
	mLinks.emplace_back(std::move(ptr));
}

//////////////////////////////////////////////////////////////////////////

/**
@brief Creates and binds a float parameter before adding it to the group
**/
static OFAbstractParamAttrLink* createFloatParameter(nap::AttributeBase& attr)
{
	float min = 0.0f;
	float max = 1.0f;
	if (attr.getTypeInfo().isKindOf(RTTI_OF(nap::NumericAttribute<float>)))
	{
		nap::NumericAttribute<float>& c_attr = static_cast<nap::NumericAttribute<float>&>(attr);
		min = c_attr.getMin();
		max = c_attr.getMax();
	}
	else
	{
		nap::Logger::warn("float attribute %s is not of type NumericAttribute, setting adding default range", attr.getName().c_str());
	}

	// Create parameter
	nap::Attribute<float>& c_attr = static_cast<nap::Attribute<float>&>(attr);
	ofParameter<float> parameter(c_attr.getName(), c_attr.getValue(), min, max);

	// Create new link (TODO: MAKE UNIQUE PTR)
	OFParamAttrLink<float>* link = new OFParamAttrLink<float>(parameter, c_attr);
	return link;
}


/**
@brief Creates and binds an int parameter
**/
static OFAbstractParamAttrLink* createIntParameter(nap::AttributeBase& attr)
{
	int min = 0;
	int max = 10;
	if (attr.getTypeInfo().isKindOf(RTTI_OF(nap::NumericAttribute<int>)))
	{
		nap::NumericAttribute<int>& c_attr = static_cast<nap::NumericAttribute<int>&>(attr);
		min = c_attr.getMin();
		max = c_attr.getMax();
	}
	else
	{
		nap::Logger::warn("int attribute :%s is not of type NumericAttribute, setting adding default range", attr.getName().c_str());
	}

	// Create new parameter
	nap::Attribute<int>& c_attr = static_cast<nap::Attribute<int>&>(attr);
	ofParameter<int> parameter(c_attr.getName(), c_attr.getValue(), min, max);

	// Create new link between parameter and attribute (TODO: MAKE UNIQUE PTR)
	OFParamAttrLink<int>* link = new OFParamAttrLink<int>(parameter, c_attr);
	return link;
}


/**
@brief Creates and binds an int parameter
**/
static OFAbstractParamAttrLink* createToggle(nap::AttributeBase& attr)
{
	// Create parameter
	nap::Attribute<bool>& c_attr = static_cast<nap::Attribute<bool>&>(attr);
	ofParameter<bool> parameter(c_attr.getName(), c_attr.getValue());

	// Create link (TODO: MAKE UNIQUE PTR)
	OFParamAttrLink<bool>* link = new OFParamAttrLink<bool>(parameter, c_attr);
	return link;
}


/**
@brief Registers all the parameter create functions
**/
void OFAttributeWrapper::sRegisterParamCreateFunctions()
{
	OFAttributeWrapper::sCreationMap.clear();
	sCreationMap[RTTI_OF(nap::Attribute<float>)] = createFloatParameter;
	sCreationMap[RTTI_OF(nap::Attribute<int>)]   = createIntParameter;
	sCreationMap[RTTI_OF(nap::Attribute<bool>)]  = createToggle;
}


/**
@brief Adds a parameter to the group, registers functions if not done before
**/
OFAbstractParamAttrLink* OFAttributeWrapper::sCreateLinkedParameter(nap::AttributeBase& attribute)
{
	// TODO, MAKE THREAD SAFE
	static bool sRegistered = false;
	if (!sRegistered)
	{
		OFAttributeWrapper::sRegisterParamCreateFunctions();
		sRegistered = true;
	}

	OFAddParameterFunction func = nullptr;
	for (auto& kv : sCreationMap)
	{
		if(attribute.getTypeInfo().isKindOf(kv.first))
		{
			func = kv.second;
			break;
		}
	}

	// Find creation function
	if (func == nullptr)
	{
		nap::Logger::warn("no attribute to OF parameter conversion function found for type: %s", attribute.getTypeInfo().getName().c_str());
		return nullptr;
	}

	// Add
	return func(attribute);
}

