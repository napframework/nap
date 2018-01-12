#pragma once

#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <rttr/type>
#include <generic/propertypath.h>

namespace napkin
{

	/**
	 * This delegate enables an QAbstractItemView to display custom editors for NAP types.
	 */
	class PropertyValueItemDelegate : public QStyledItemDelegate
	{
	public:
		/**
		 * Get the type this modelindex represents
		 * @param idx The model index that 'points' to an object with that type
		 * @return The type represented by the index
		 */
		rttr::type getTypeFromModelIndex(const QModelIndex& idx) const;

		/**
		 * Given a QModelIndex, retrieve the associated PropertyPath
		 * @param idx The index that is associated with the path.
		 * @return The path associated with the index
		 */
		const PropertyPath getPropertyPathFromIndex(const QModelIndex& idx) const;

		/**
		 * Override from QStyledItemDelegate
		 */
		QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
							  const QModelIndex& index) const override;

		/**
		 * Override from QStyledItemDelegate
		 */
		void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

		/**
		 * Override from QStyledItemDelegate
		 */
		void setEditorData(QWidget* editor, const QModelIndex& index) const override;

		/**
		 * Override from QStyledItemDelegate
		 */
		void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

		/**
		 * Override from QStyledItemDelegate
		 */
		QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

	protected:
		/**
		 * Override from QStyledItemDelegate
		 */
		bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
						 const QModelIndex& index) override;
	};
};