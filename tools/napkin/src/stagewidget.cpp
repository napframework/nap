#include "stagewidget.h"
#include "appcontext.h"
#include "naputils.h"
#include <nap/projectinfo.h>

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
	{ }


	bool StageWidget::loadPath(const PropertyPath& path, nap::utility::ErrorState& error)
	{
		assert(path.getObject() != nullptr);
		assert(toOption().isCompatible(path.getObject()->get_type()));
		return onLoadPath(path, error);
	}


	bool StageWidget::isSupported(const nap::ProjectInfo& info) const
	{
		return !getTypes([this](const auto& ctype) 
			{
				for (const auto& stype : mTypes)
				{
					if (ctype.is_derived_from(stype))
						return true;
				} return false;
			}).empty();
	}
}
