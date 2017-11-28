#include "utility.h"

qreal lerp(const qreal& a, const qreal& b, qreal p)
{ return a + (b - a) * p; }

QColor lerpCol(const QColor& a, const QColor& b, qreal p)
{
    QColor c;
    c.setRgbF(lerp(a.redF(), b.redF(), p), lerp(a.greenF(), b.greenF(), p), lerp(a.blueF(), b.blueF(), p));
    return c;
}

const QColor& softForeground()
{
    static QColor c = lerpCol(QApplication::palette().color(QPalette::Normal, QPalette::WindowText),
                              QApplication::palette().color(QPalette::Disabled, QPalette::WindowText), 0.5);
    return c;
}

const QColor& softBackground()
{
    static QColor c = QApplication::palette().color(QPalette::Normal, QPalette::Window).darker(102);
    return c;
}

bool traverse(const QAbstractItemModel& model, std::function<bool(const QModelIndex&)> visitor, QModelIndex parent)
{
    for (int r = 0; r < model.rowCount(parent); r++)
    {
        auto index = model.index(r, 0, parent);
        if (!visitor(index))
            return false;
        if (model.hasChildren(index))
            if (!traverse(model, visitor, index))
                return false;
    }
    return true;
}

QModelIndex findItemInModel(const QAbstractItemModel& model, std::function<bool(const QModelIndex& idx)> condition)
{
    QModelIndex foundIndex;

    traverse(model, [&foundIndex, condition](const QModelIndex& idx) -> bool {
        if (condition(idx))
        {
            foundIndex = idx;
            return false;
        }
        return true;
    });

    return foundIndex;
}

std::vector<rttr::type> getComponentTypes()
{
    std::vector<rttr::type> ret;
    nap::rtti::TypeInfo rootType = RTTI_OF(nap::Component);
    for (const nap::rtti::TypeInfo& derived : rootType.get_derived_classes())
    {
        if (derived.can_create_instance())
            ret.emplace_back(derived);
    }
    return ret;
}

std::vector<rttr::type> getResourceTypes()
{
    // TODO: Find a proper way to retrieve 'resource types' via RTTI
    std::vector<rttr::type> ret;
    rttr::type rootType = RTTI_OF(nap::rtti::RTTIObject);
    for (const rttr::type& derived : rootType.get_derived_classes())
    {
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
