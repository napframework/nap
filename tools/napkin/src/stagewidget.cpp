#include "stagewidget.h"
#include "appcontext.h"

namespace napkin
{
	bool StageOption::isCompatible(const nap::rtti::TypeInfo& otherType) const
	{
		for (const auto& type : mTypes)
		{
			if (otherType.is_derived_from(type))
				return true;
		}
		return false;
	}


	StageWidget::StageWidget(std::string&& displayName, StageOption::Types&& types, QWidget* parent /*= nullptr*/) : QWidget(parent),
		mDisplayName(displayName), mTypes(types)
	{
		connect(&AppContext::get(), &AppContext::propertyValueChanged, this, &StageWidget::onPropertyValueChanged);
		connect(&AppContext::get(), &AppContext::objectRemoved, this, &StageWidget::onObjectRemoved);
	}


	void StageWidget::setPath(const PropertyPath& path)
	{
		assert(path.getObject() != nullptr);
		assert(toOption().isCompatible(path.getObject()->get_type()));
		mObject = path.getObject();
		loadPath(path);
	}


	void StageWidget::onPropertyValueChanged(const PropertyPath& path)
	{
		// Reload if property of currently staged object changed
		if (mObject == path.getObject())
			loadPath(path);
	}


	void StageWidget::onObjectRemoved(nap::rtti::Object* object)
	{
		if (mObject == object)
		{
			mObject = nullptr;
			clearPath();
		}
	}
}
