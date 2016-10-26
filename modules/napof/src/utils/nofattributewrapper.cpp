// Local Includes
#include "nofattributewrapper.h"
#include "napofattributes.h"

// External Includes
#include <nap/coreattributes.h>
#include <ofColor.h>

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
	// Create parameter
	nap::NumericAttribute<float>& c_attr = static_cast<nap::NumericAttribute<float>&>(attr);
	ofParameter<float> parameter(c_attr.getName(), c_attr.getValue(), c_attr.getMin(), c_attr.getMax());

	// Create new link (TODO: MAKE UNIQUE PTR)
	return new OFParamNumericAttrLink<float>(parameter, c_attr);
}


/**
@brief Creates and binds an int parameter
**/
static OFAbstractParamAttrLink* createIntParameter(nap::AttributeBase& attr)
{
	// Create new parameter
	nap::NumericAttribute<int>& c_attr = static_cast<nap::NumericAttribute<int>&>(attr);
	ofParameter<int> parameter(c_attr.getName(), c_attr.getValue(), c_attr.getMin(), c_attr.getMax());

	// Create new link between parameter and attribute (TODO: MAKE UNIQUE PTR)
	return new OFParamNumericAttrLink<int>(parameter, c_attr);
}


/**
@brief Creates and binds an int parameter
**/
static OFAbstractParamAttrLink* createToggle(nap::AttributeBase& attr)
{
	// Create parameter
	nap::Attribute<bool>& c_attr = static_cast<nap::Attribute<bool>&>(attr);
	ofParameter<bool> parameter(c_attr.getName(), c_attr.getValue());

	// Create link
	return new OFParamAttrLink<bool>(parameter, c_attr);
}


/**
@brief Create float color
**/
static OFAbstractParamAttrLink* createFloatColor(nap::AttributeBase& attr)
{
	nap::Attribute<ofFloatColor>& c_attr = static_cast<nap::Attribute<ofFloatColor>&>(attr);
	ofParameter<ofFloatColor> parameter(c_attr.getName(), c_attr.getValue());

	// Create link
	return new OFParamAttrLink<ofFloatColor>(parameter, c_attr);
}


/**
@brief Create ofVec4f
**/
static OFAbstractParamAttrLink* createofVec4f(nap::AttributeBase& attr)
{
	nap::NumericAttribute<ofVec4f>& c_attr = static_cast<nap::NumericAttribute<ofVec4f>&>(attr);
	ofParameter<ofVec4f> parameter(c_attr.getName(), c_attr.getValue(), c_attr.getMin(), c_attr.getMax());
	return new OFParamNumericAttrLink<ofVec4f>(parameter, c_attr);
}


/**
@brief Create ofVec3f
**/
static OFAbstractParamAttrLink* createofVec3f(nap::AttributeBase& attr)
{
	nap::NumericAttribute<ofVec3f>& c_attr = static_cast<nap::NumericAttribute<ofVec3f>&>(attr);
	ofParameter<ofVec3f> parameter(c_attr.getName(), c_attr.getValue(), c_attr.getMin(), c_attr.getMax());
	return new OFParamNumericAttrLink<ofVec3f>(parameter, c_attr);
}


/**
@brief Create ofVec2f
**/
static OFAbstractParamAttrLink* createofVec2f(nap::AttributeBase& attr)
{
	nap::NumericAttribute<ofVec2f>& c_attr = static_cast<nap::NumericAttribute<ofVec2f>&>(attr);
	ofParameter<ofVec2f> parameter(c_attr.getName(), c_attr.getValue(), c_attr.getMin(), c_attr.getMax());
	return new OFParamNumericAttrLink<ofVec2f>(parameter, c_attr);
}


/**
@brief creates a label
**/
static OFAbstractParamAttrLink* createLabel(nap::AttributeBase& attr)
{
	nap::Attribute<std::string>& c_attr = static_cast<nap::Attribute<std::string>&>(attr);
	ofParameter<std::string> parameter(c_attr.getName(), c_attr.getValue());
	return new OFParamAttrLink<std::string>(parameter, c_attr);
}


/**
@brief Creates a button
**/
static OFAbstractParamAttrLink* createButton(nap::AttributeBase& attr)
{
	nap::SignalAttribute& c_attr = static_cast<nap::SignalAttribute&>(attr);
	ofParameter<bool> paramter(c_attr.getName(), false);
	return new OFParamSignalLink(paramter, c_attr);
}


/**
@brief Registers all the parameter create functions
**/
void OFAttributeWrapper::sRegisterParamCreateFunctions()
{
	OFAttributeWrapper::sCreationMap.clear();
	sCreationMap[RTTI_OF(nap::NumericAttribute<float>)] = createFloatParameter;
	sCreationMap[RTTI_OF(nap::NumericAttribute<int>)]   = createIntParameter;
	sCreationMap[RTTI_OF(nap::Attribute<bool>)]  = createToggle;
	sCreationMap[RTTI_OF(nap::Attribute<ofFloatColor>)] = createFloatColor;
	sCreationMap[RTTI_OF(nap::NumericAttribute<ofVec2f>)] = createofVec2f;
	sCreationMap[RTTI_OF(nap::NumericAttribute<ofVec3f>)] = createofVec3f;
	sCreationMap[RTTI_OF(nap::NumericAttribute<ofVec4f>)] = createofVec4f;
	sCreationMap[RTTI_OF(nap::Attribute<std::string>)] = createLabel;
	sCreationMap[RTTI_OF(nap::SignalAttribute)] = createButton;
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

