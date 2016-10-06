#include <nofparamattrlink.h>
#include <nap/attributeobject.h>

OFAbstractParamAttrLink::OFAbstractParamAttrLink(ofAbstractParameter& param, nap::AttributeBase& attrib) :
	mAttribute(&attrib)
{
	// Store reference to parameter
	mParameter = param.newReference();

	// When the attribute is removed, we want to make sure we don't listen to changes anymore
	mAttribute->getParent()->removed.connect(mAttributeRemoved);
}



OFAbstractParamAttrLink::~OFAbstractParamAttrLink()
{}


/**
@brief Returns the name of this attribute link, empty if not bound
**/
std::string OFAbstractParamAttrLink::getName() const
{
	return mAttribute == nullptr ? "" : mAttribute->getName();
}


/**
@brief Sets a new attribute to link with an ofParameter
**/
void OFAbstractParamAttrLink::setAttribute(nap::AttributeBase& attribute)
{
	if (mAttribute)
	{
		// now make sure we listen to remove calls of this new one
		mAttribute->removed.disconnect(mAttributeRemoved);

		// Call derived class to clean possible connections
		attributeChanged(attribute);
	}

	mAttribute = &attribute;
	mAttribute->getParent()->removed.connect(mAttributeRemoved);
}


/**
@brief Call child class and clear attribute link
**/
void OFAbstractParamAttrLink::attributeRemoved(nap::Object&)
{
	stopListening();
	mAttribute = nullptr;
}


/**
@brief Parameter signal link
**/
OFParamSignalLink::OFParamSignalLink(ofParameter<bool>& param, nap::SignalAttribute& attribute) : OFAbstractParamAttrLink(param, attribute)
{
	param.addListener(this, &OFParamSignalLink::parameterValueChanged);
}


/**
@brief Stop listening to changes from toggle, ie, don't forward changes
**/
void OFParamSignalLink::stopListening()
{
	getParameter()->removeListener(this, &OFParamSignalLink::parameterValueChanged);
}



/**
@brief We don't actually listen to the attribute, ie, we don't show changes in the toggle
**/
void OFParamSignalLink::attributeChanged(nap::AttributeBase& new_attr)
{
	return;
}


/**
@brief Occurs when the boolean value changes
**/
void OFParamSignalLink::parameterValueChanged(bool& value)
{
	if (!value)
		return;

	ofParameter<bool>* parameter = getParameter();
	if (parameter == nullptr)
	{
		assert(false);
		return;
	}

	// Trigger associated attribute
	getSignalAttribute()->trigger();

	// Set parameter back to false
	parameter->set(false);
}


/**
@brief Returns the signal attribute
**/
nap::SignalAttribute* OFParamSignalLink::getSignalAttribute()
{
	return mAttribute == nullptr ? nullptr : static_cast<nap::SignalAttribute*>(mAttribute);
}


/**
@brief Casts stored parameter to bool parameter
**/
ofParameter<bool>* OFParamSignalLink::getParameter()
{
	return mParameter == nullptr ? nullptr : static_cast<ofParameter<bool>*>(mParameter.get());
}
