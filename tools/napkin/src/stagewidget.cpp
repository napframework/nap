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
		//connect(&AppContext::get(), &AppContext::propertyValueChanged, this, &StageWidget::onPropertyValueChanged);
		//connect(&AppContext::get(), &AppContext::objectRemoved, this, &StageWidget::onObjectRemoved);
	}


	bool StageWidget::loadPath(const PropertyPath& path, nap::utility::ErrorState& error)
	{
		assert(path.getObject() != nullptr);
		assert(toOption().isCompatible(path.getObject()->get_type()));
		return onLoadPath(path, error);
	}
}
