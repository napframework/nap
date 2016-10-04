#pragma once

// OF Includes
#include <ofParameterGroup.h>
#include <Utils/nofparamattrlink.h>

// Nap Includes
#include <nap/attributeobject.h>
#include <rtti/rtti.h>

// Std Includes
#include <unordered_map>


/**
@brief Wraps a set of attributes to OFParameters

This object populates an ofParameterGroup with parameters mapped to attributes
Changes are automatically forwarded from parameter to attribute and the other way round
Use the getGroup() method to fetch all the parameters to populate a ui element in OF
**/

using OFAddParameterFunction = std::function<OFAbstractParamAttrLink*(nap::AttributeBase&)>;
using OFParameterMap = std::unordered_map<RTTI::TypeInfo, OFAddParameterFunction>;

class OFAttributeWrapper
{
public:
	// Constructors
	OFAttributeWrapper() = default;
	OFAttributeWrapper(const std::string& groupName) { setName(groupName); }

	// Attributes
	void							addObject(nap::AttributeObject& object);
	void							addAttribute(nap::AttributeBase& attribute);

	// Getters
	const ofParameterGroup&			getGroup() const { return mGroup; }

	// Setters
	void							setName(const std::string& name) { mGroup.setName(name); }

	// Statics
	static OFParameterMap			sCreationMap;
	static OFAbstractParamAttrLink*	sCreateLinkedParameter(nap::AttributeBase& attribute);

private:
	static void						sRegisterParamCreateFunctions();

	// Populated parameter group
	ofParameterGroup mGroup;

	// All the created and managed links
	using LinkedParameters = std::vector<std::unique_ptr<OFAbstractParamAttrLink>>;
	LinkedParameters mLinks;
};


//////////////////////////////////////////////////////////////////////////

