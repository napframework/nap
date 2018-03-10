#pragma once

#include <QtWidgets/QDialog>
#include <rtti/rttiobject.h>
#include "filtertreeview.h"

namespace napkin
{
	class FlatObjectModel : public QStandardItemModel
	{
	public:
		FlatObjectModel(const rttr::type& basetype);

	private:
		const rttr::type mBaseType;
	};

	class FlatTypeModel : public QStandardItemModel
	{
	public:
		FlatTypeModel(const rttr::type& basetype);
	private:
		const rttr::type mBaseType;

	};

	/**
	 * General purpose popup dialog showing a filterable tree.
	 */
	class FilterPopup : public QMenu
	{
	public:
		explicit FilterPopup(QWidget* parent, QStandardItemModel& model);

	public:

		/**
		 * Display a selection dialog with all available objects, filtered by a base type
		 * @param parent The parent widget to attach to.
		 * @param typeConstraint The base type to filter by.
		 * @return The selected object or nullptr if no object was selected
		 */
		static nap::rtti::RTTIObject* getObject(QWidget* parent, const rttr::type& typeConstraint);

		/**
		 * Display a selection dialog with all available instantiable subtypes of the specified base type.
		 * @param parent The parent widget to attach to
		 * @param baseType The base type to constrain the selection to.
		 * @return The resulting selected type or an empty/invalid type if none was selected.
		 */
		static nap::rtti::TypeInfo getDerivedType(QWidget* parent, const rttr::type& baseType);


		/**
		 * Display a selection dialog with all available types that can be added as a resource.
		 * @param parent The widget to attach to
		 * @return The resulting selected type.
		 */
		static nap::rtti::TypeInfo getResourceType(QWidget* parent);

		/**
		 * Display a selection dialog with all available objects, filtered by type T
		 * @tparam T the base type to filter by
		 * @param parent The parent widget to attach to.
		 * @return The selected object or nullptr if no object was selected
		 */
		template<typename T>
		static T* getObject(QWidget* parent) { return rtti_cast<T>(getObject(parent, RTTI_OF(T))); }

		QSize sizeHint() const override;

	protected:
		/**
		 * Set focus etc
		 */
		void showEvent(QShowEvent* event) override;

		/**
		 * Capture keyboard for confirmation etc
		 */
		void keyPressEvent(QKeyEvent* event) override;

	private:
		void moveSelection(int dir);
		void confirm();


		FilterTreeView mTreeView;
		QVBoxLayout mLayout;
		QSize mSize = { 400, 400 };
	};

} // namespace napkin