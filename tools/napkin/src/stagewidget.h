#pragma once

// Local includes
#include "propertypath.h"

// External includes
#include <QWidget>
#include <QList>
#include <rtti/typeinfo.h>
#include <nap/projectinfo.h>

namespace napkin
{
	/**
	 * Loose coupling between a set of types and compatible stage widget.
	 * An option binds a set of resource types to a widget that can handle them.
	 */
	struct StageOption
	{
		using Types = std::vector<nap::rtti::TypeInfo>;

		/**
		 * Called by StageWidget::toOption()
		 * @param widgetName widget object name
		 * @param displayName widget display name
		 * @param types list of compatible types
		 * @param icon stage icon
		 */
		StageOption(const std::string& widgetName, const std::string& displayName, const Types& types, const Types& excludeTypes, const QIcon& icon) :
			mWidgetName(widgetName), mDisplayName(displayName), mTypes(types), mExcludeTypes(excludeTypes), mIcon(icon) {}

		std::string mDisplayName;	///< Widget display name
		std::string mWidgetName;	///< Widget object name
		Types mTypes;				///< Available preview types
		Types mExcludeTypes;		///< Exclude preview types
		QIcon mIcon;				///< Stage icon

		/**
		 * Checks if the given type can be staged (previewed) by this option.
		 * @param otherType the type to check for compatibility 
		 * @return if the given type can be staged by this option.
		 */
		bool isCompatible(const nap::rtti::TypeInfo& otherType) const;

		/**
		 * @return if this option refers to the same widget
		 */
		bool operator== (const StageOption& rhs) const { return rhs.mWidgetName == this->mWidgetName; }

		/**
		 * @return if this option does not refer to the same widget
		 */
		bool operator!=(const StageOption& rhs) const { return !(rhs == *this); }
	};


	/**
	 * NAP resource preview widget.
	 */
	class StageWidget : public QWidget
	{
		Q_OBJECT
	public:
		/**
		 * @param displayName widget display name
		 * @param types compatible types
		 * @param parent widget parent
		 */
		StageWidget(std::string&& displayName, StageOption::Types&& types, StageOption::Types&& excludeTypes, nap::rtti::TypeInfo&& iconType, QWidget* parent = nullptr);

		/**
		 * Converts this staging widget into an option that can be used to find and load matching types. 
		 * @return Staging option
		 */
		StageOption toOption() const;

		/**
		 * @return Widget display name
		 */
		std::string getDisplayName() const				{ return mDisplayName; }

		/**
		 * Set path to load, note that the past must be validated as an option
		 * @param path the path to load
		 */
		bool loadPath(const PropertyPath& path, nap::utility::ErrorState& error);

		/**
		 * Checks if this widget supports types from the given project.
		 * Note that this call is not fast and shouldn't be called frequently.
		 * @return if the staging widget can be used with the given project
		 */
		bool isSupported(const nap::ProjectInfo& info) const;

	protected:
		// Implement in derived classes to load validated path
		virtual bool onLoadPath(const PropertyPath& path, nap::utility::ErrorState& error) = 0;

	private:
		std::string mDisplayName;
		StageOption::Types mTypes;
		StageOption::Types mExcludeTypes;
		nap::rtti::TypeInfo mIconType;
	};
}


//////////////////////////////////////////////////////////////////////////
// StageOption hash function
//////////////////////////////////////////////////////////////////////////
namespace std
{
	template<>
	struct hash <napkin::StageOption>
	{
		size_t operator()(const napkin::StageOption& v) const
		{
			return hash<std::string>()(v.mWidgetName);
		}
	};
}
