#pragma once

// OF Includes
#include <ofParameterGroup.h>

// Nap Includes
#include <nap/attributeobject.h>

/**
@brief Wraps a set of attributes to OFParameters

This object populates an ofParameterGroup with parameters mapped to attributes
Changes are automatically forwarded from parameter to attribute and the other way round
Use the getGroup() method to fetch all the parameters to populate a ui element in OF
**/
class OFAttributeWrapper
{
public:
	// Constructors
	OFAttributeWrapper() = default;
	OFAttributeWrapper(const std::string& groupName) { setName(groupName); }

	// Attributes
	void addObject(nap::AttributeObject& object);
	void addAttribute(nap::AttributeBase& attribute);

	// Getters
	const ofParameterGroup& getGroup() const { return mGroup; }

	// Setters
	void setName(const std::string& name) { mGroup.setName(name); }

private:
	ofParameterGroup mGroup;
};
