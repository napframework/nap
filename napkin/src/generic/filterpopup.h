#pragma once

#include <QtWidgets/QDialog>
#include <rtti/object.h>
#include "filtertreeview.h"
#include "naputils.h"

namespace napkin
{
	class FlatObjectModel : public QStandardItemModel
	{
	public:
		FlatObjectModel(const rttr::type& basetype);

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
		static nap::rtti::Object* getObject(QWidget* parent, const rttr::type& typeConstraint);

		/**
		 * Display a selection dialog with all available types, filtered by an optional predicate
		 * @param parent The widget to attach to
		 * @return The resulting selected type.
		 */
		static nap::rtti::TypeInfo getType(QWidget* parent, const TypePredicate& predicate = nullptr);

		/**
		 * Display a selection dialog with all available objects, filtered by type T
		 * @tparam T the base type to filter by
		 * @param parent The parent widget to attach to.
		 * @return The selected object or nullptr if no object was selected
		 */
		template<typename T>
		static T* getObject(QWidget* parent) { return rtti_cast<T>(getObject(parent, RTTI_OF(T))); }

		/**
		 * Override to provide a reasonable size
		 */
		QSize sizeHint() const override;

		/**
		 * @return true if the user choice was confirmed, false if the dialog was dismissed
		 */
		bool wasAccepted() const { return mWasAccepted; }

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

		bool mWasAccepted = false;
		FilterTreeView mTreeView;
		QVBoxLayout mLayout;
		QSize mSize = { 400, 400 };

	};

} // namespace napkin