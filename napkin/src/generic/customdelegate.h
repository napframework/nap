#pragma once


#include <QItemDelegate>
#include <QWidget>

class CustomDelegate : public QItemDelegate {
public:
    QWidget*
    createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        return QItemDelegate::createEditor(parent, option, index);
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        QItemDelegate::setEditorData(editor, index);
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
        QItemDelegate::setModelData(editor, model, index);
    }

    void
    updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QItemDelegate::updateEditorGeometry(editor, option, index);
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QItemDelegate::paint(painter, option, index);
    }


};
