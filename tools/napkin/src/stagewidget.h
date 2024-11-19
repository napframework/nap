#pragma once

#include <QWidget>
#include <QList>
#include <rtti/typeinfo.h>

namespace napkin
{
	/**
	 * Loose coupling between a set of types and compatible StageWidget.
	 */
	struct StageOption
	{
		using Types = std::vector<nap::rtti::TypeInfo>;

		/**
		 * Called by StageWidget::toOption()
		 * @param widgetName widget object name 
		 * @param displayName widget display name
		 * @param types list of compatible types
		 */
		StageOption(const std::string&& widgetName, const std::string&& displayName, const Types& types) :
			mWidgetName(widgetName), mDisplayName(displayName), mTypes(types) { }

		std::string mDisplayName;	///< Display name
		std::string mWidgetName;	///< Unique stage widget name
		Types mTypes;				///< Available preview types

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
		StageWidget(QWidget* parent = nullptr) : QWidget(parent)		{ }

		/**
		 * Override in derived classes: returns the accepted nap object types for this staging widget. 
		 * @return accepted NAP preview types
		 */
		virtual std::vector<nap::rtti::TypeInfo> getTypes() const = 0;

		/**
		 * @return Display name
		 */
		virtual QString getDisplayName() const = 0;

		/**
		 * Converts this staging widget into an option that can be used to find and load matching types. 
		 * @return Staging option
		 */
		StageOption toOption() const;
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
