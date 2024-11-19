#pragma once

#include <QWidget>
#include <QList>
#include <rtti/typeinfo.h>

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
		 */
		StageOption(const std::string& widgetName, const std::string& displayName, const Types& types) :
			mWidgetName(widgetName), mDisplayName(displayName), mTypes(types) { }

		std::string mDisplayName;	///< Widget display name
		std::string mWidgetName;	///< Widget object name
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
		/**
		 * @param displayName widget display name
		 * @param types compatible types
		 * @param parent widget parent
		 */
		StageWidget(std::string&& displayName, StageOption::Types&& types, QWidget* parent = nullptr) : QWidget(parent),
			mDisplayName(displayName), mTypes(types)		{ }

		/**
		 * Converts this staging widget into an option that can be used to find and load matching types. 
		 * @return Staging option
		 */
		StageOption toOption() const					{ return StageOption(objectName().toStdString(), mDisplayName, mTypes); }

		/**
		 * @return Widget display name
		 */
		std::string getDisplayName() const				{ return mDisplayName; }

	private:
		std::string mDisplayName;
		StageOption::Types mTypes;
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
