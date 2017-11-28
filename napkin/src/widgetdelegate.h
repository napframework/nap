#pragma once

#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <rttr/type>


/**
 * This delegate enables an QAbstractItemView to display custom editors for NAP types.
 */
class PropertyValueItemDelegate : public QStyledItemDelegate
{
public:
	rttr::type typeFor(const QModelIndex& idx) const;

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	void setEditorData(QWidget* editor, const QModelIndex& index) const override;
	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

protected:
	bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
					 const QModelIndex& index) override;

public:
	QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};