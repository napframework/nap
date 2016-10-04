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
	attributeDisconnected();
	mAttribute = nullptr;
}
