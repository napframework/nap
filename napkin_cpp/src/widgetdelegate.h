#pragma once


#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include <rttr/type>

class TypedDelegate {
public:
    virtual void
    paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, rttr::type type) = 0;

    virtual bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) = 0;

    virtual bool isValidFor(rttr::type type) = 0;

};

class TypedDelegateBool : public TypedDelegate {
public:
    void
    paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, rttr::type type) override;

    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

    bool isValidFor(rttr::type type) override;
};

class TypedDelegateEnum : public TypedDelegate {
public:
    void
    paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, rttr::type type) override;

    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

    bool isValidFor(rttr::type type) override;
};

class WidgetDelegate : public QStyledItemDelegate {
public:
    WidgetDelegate();

    ~WidgetDelegate();

    rttr::type typeFor(const QModelIndex& idx) const;
    TypedDelegate* delegateFor(const QModelIndex& idx) const;
    TypedDelegate* delegateFor(rttr::type type) const;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

protected:
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
                     const QModelIndex& index) override;

public:
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    std::vector<TypedDelegate*> mDelegates;
};