#include "stagewidget.h"
#include "appcontext.h"
#include "naputils.h"
#include <nap/projectinfo.h>

namespace napkin
{
	bool StageOption::isCompatible(const nap::rtti::TypeInfo& otherType) const
	{
		// Check if it's derived and not excluded
		for (const auto& type : mTypes)
		{
			if (otherType.is_derived_from(type))
			{
				auto excl_it = std::find_if(mExcludeTypes.begin(), mExcludeTypes.end(), [&otherType](const auto& excl_type)  {
					return otherType == excl_type;
				});
				return excl_it == mExcludeTypes.end();
			}
		}
		return false;
	}


	StageWidget::StageWidget(std::string&& displayName, StageOption::Types&& types, StageOption::Types&& excludeTypes, nap::rtti::TypeInfo&& iconType, QWidget* parent) : QWidget(parent),
		mDisplayName(displayName), mTypes(types), mExcludeTypes(excludeTypes), mIconType(iconType)
	{ }


	napkin::StageOption StageWidget::toOption() const
	{
		return
		{
			objectName().toStdString(),
			mDisplayName,
			mTypes,
			mExcludeTypes,
			AppContext::get().getResourceFactory().getIcon(mIconType)
		};
	}


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

