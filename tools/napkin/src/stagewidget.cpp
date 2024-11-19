#include "stagewidget.h"

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


	napkin::StageOption StageWidget::toOption() const
	{
		return StageOption(objectName().toStdString(),
			getDisplayName().toStdString(), getTypes());
	}
}
