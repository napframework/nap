#include "attributecontroller.h"
#include "statecontroller.h"


RTTI_DEFINE(nap::AttrCtrl)

namespace nap
{
    AttrCtrl::AttrCtrl() {}

	void AttrCtrl::activate()
	{
		nap::AttributeBase* attrib = getTargetAttribute();
		assert(attrib);
		attrib->setValue(*getControlAttribute());
	}

	void AttrCtrl::setTargetAttribute(nap::AttributeBase& attr)
	{
		if (!mTargetAttribute)
		{
			nap::Link& new_link = this->addChild<nap::Link>("controlTargetAttribute");
			new_link.setTarget(attr);
			mTargetAttribute = &new_link;
		}

		if (mControlAttribute) nap::AttributeObject::removeAttribute(*mControlAttribute);
		mControlAttribute = &nap::AttributeObject::addAttribute("ControlValue", attr.getTypeInfo());
	}

	AttrCtrl::AttrCtrl(AttributeBase &targetAttribute, const StateMode &trigger) : StateCtrl() {
		setTrigger(trigger);
		setName(targetAttribute.getName() + "_controller");
		setTargetAttribute(targetAttribute);
	}



}