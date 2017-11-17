#pragma once

#include <QColor>
#include <QApplication>
#include <QPalette>
#include <QAbstractItemModel>
#include <QtGui/QStandardItemModel>
#include <QMetaEnum>
#include <functional>
#include <outlinepanel.h>

static qreal lerp(const qreal& a, const qreal& b, qreal p)
{
    return a + (b - a) * p;
}

static QColor lerpCol(const QColor& a, const QColor& b, qreal p)
{
    QColor c;
    c.setRgbF(lerp(a.redF(), b.redF(), p),
              lerp(a.greenF(), b.greenF(), p),
              lerp(a.blueF(), b.blueF(), p));
    return c;
}

static const QColor& softForeground()
{
    static QColor c = lerpCol(QApplication::palette().color(QPalette::Normal, QPalette::WindowText),
                              QApplication::palette().color(QPalette::Disabled, QPalette::WindowText), 0.5);
    return c;
}

static const QColor& softBackground()
{
    static QColor c = QApplication::palette().color(QPalette::Normal, QPalette::Window).darker(102);
    return c;
}

/**
 * Recursively traverse the provided model and invoke the provided visitor for every item.
 * @param model The mod
 * @param parent The root index to start at
 * @param visitor The function to invoke on every item, return true if the traversal should continue
 * @return false if the the visitor has decided to stop traversal
 */
static bool
traverse(const QAbstractItemModel& model, std::function<bool(const QModelIndex&)> visitor, QModelIndex parent = QModelIndex())
{
    for (int r = 0; r < model.rowCount(parent); r++) {
        auto index = model.index(r, 0, parent);
        if (!visitor(index))
            return false;
        if (model.hasChildren(index))
            if (!traverse(model, visitor, index))
                return false;
    }
    return true;
}

static QModelIndex findItemInModel(const QAbstractItemModel& model, std::function<bool(const QModelIndex& idx)> condition)
{
    QModelIndex foundIndex;

    traverse(model, [&foundIndex, condition](const QModelIndex& idx) -> bool {
        if (condition(idx)) {
            foundIndex = idx;
            return false;
        }
        return true;
    });

    return foundIndex;
}



static std::vector<rttr::type> getComponentTypes()
{
    std::vector<rttr::type> ret;
    nap::rtti::TypeInfo rootType = RTTI_OF(nap::Component);
    for (const nap::rtti::TypeInfo& derived : rootType.get_derived_classes()) {
        if (derived.can_create_instance())
            ret.emplace_back(derived);
    }
    return ret;
}

static std::vector<rttr::type> getResourceTypes()
{
    // TODO: Find a proper way to retrieve 'resource types' via RTTI
    std::vector<rttr::type> ret;
    rttr::type rootType = RTTI_OF(nap::rtti::RTTIObject);
    for (const rttr::type& derived : rootType.get_derived_classes()) {
        if (derived.is_derived_from<nap::Component>())
            continue;
        if (derived.is_derived_from<nap::Entity>())
            continue;
        if (derived.is_derived_from<nap::ComponentInstance>())
            continue;
        if (!derived.can_create_instance())
            continue;

        ret.emplace_back(derived);
    }
    return ret;
}

template<typename QEnum>
static QString QEnumToString (const QEnum value)
{
    return QMetaEnum::fromType<QEnum>().valueToKey(value);
}