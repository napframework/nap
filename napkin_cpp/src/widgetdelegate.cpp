#include "widgetdelegate.h"
#include "inspectorpanel.h"
#include "typeconversion.h"
#include <rtti/typeinfo.h>

WidgetDelegate::WidgetDelegate()
{
    mDelegates.emplace_back(new TypedDelegateBool);
}


WidgetDelegate::~WidgetDelegate()
{
    mDelegates.clear();
}


void WidgetDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto type = typeFor(index);
    auto delegate = delegateFor(type);
    if (delegate) {
        delegate->paint(painter, option, index, type);
        return;
    }
    QStyledItemDelegate::paint(painter, option, index);

}

QSize WidgetDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

bool WidgetDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
                                 const QModelIndex& index)
{
    auto delegate = delegateFor(index);
    if (delegate) {
        delegate->editorEvent(event, model, option, index);
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

rttr::type WidgetDelegate::typeFor(const QModelIndex& index) const
{
    auto variant = index.data(Qt::UserRole);
    if (variant.canConvert<TypeWrapper>()) {
        return *variant.value<TypeWrapper>().type;
    }
    return rttr::detail::get_invalid_type();
}

TypedDelegate* WidgetDelegate::delegateFor(const QModelIndex& index) const
{
    return delegateFor(typeFor(index));
}

TypedDelegate* WidgetDelegate::delegateFor(rttr::type type) const
{
    for (auto delegate : mDelegates) {
        if (delegate->isValidFor(type))
            return delegate;
    }
    return nullptr;
}


void TypedDelegateBool::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index,
                              rttr::type type)
{
    // Draw bool
    QStyleOptionButton styleOption;
    styleOption.rect = option.rect;
    if (index.data(Qt::DisplayRole).toBool()) {
        styleOption.state |= QStyle::State_On;
    } else {
        styleOption.state |= QStyle::State_Off;
    }
    QApplication::style()->drawControl(QStyle::CE_CheckBox, &styleOption, painter);

}

bool TypedDelegateBool::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
                                    const QModelIndex& index)
{
    if (event->type() == QEvent::MouseButtonPress) {
        bool val = index.data(Qt::DisplayRole).toBool();
        model->setData(index, !val, Qt::EditRole);
    }

    return false;
}

bool TypedDelegateBool::isValidFor(rttr::type type)
{
    return type == rttr::type::get<bool>();
}

void TypedDelegateEnum::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index,
                              rttr::type type)
{

}

bool TypedDelegateEnum::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
                                    const QModelIndex& index)
{
    return false;
}

bool TypedDelegateEnum::isValidFor(rttr::type type)
{
    return type.is_enumeration();
}
