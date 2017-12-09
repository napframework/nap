#include "utility.h"
#include <mathutils.h>


QColor napkin::lerpCol(const QColor& a, const QColor& b, qreal p)
{
	QColor c;
	c.setRgbF(nap::math::lerp(a.redF(), b.redF(), p),
              nap::math::lerp(a.greenF(), b.greenF(), p),
              nap::math::lerp(a.blueF(), b.blueF(), p));
	return c;
}

const QColor& napkin::getSoftForeground()
{
	static QColor c = lerpCol(QApplication::palette().color(QPalette::Normal, QPalette::WindowText),
							  QApplication::palette().color(QPalette::Disabled, QPalette::WindowText), 0.5);
	return c;
}

const QColor& napkin::getSoftBackground()
{
	static QColor c = QApplication::palette().color(QPalette::Normal, QPalette::Window).darker(102);
	return c;
}

bool napkin::traverse(const QAbstractItemModel& model, ModelIndexFilter visitor, QModelIndex parent, int column)
{
	for (int r = 0; r < model.rowCount(parent); r++)
	{
		auto index = model.index(r, 0, parent);
		auto colindex = model.index(r, column, parent);
		if (!visitor(colindex))
			return false;
		if (model.hasChildren(index))
			if (!traverse(model, visitor, index))
				return false;
	}
	return true;
}

bool napkin::traverse(const QStandardItemModel& model, ModelItemFilter visitor, QModelIndex parent, int column)
{
	for (int r = 0; r < model.rowCount(parent); r++)
	{
		auto index = model.index(r, 0, parent);
		auto colindex = model.index(r, column, parent);
		if (!visitor(model.itemFromIndex(colindex)))
			return false;
		if (model.hasChildren(index))
			if (!traverse(model, visitor, index))
				return false;
	}
	return true;
}

QModelIndex napkin::findIndexInModel(const QAbstractItemModel& model, ModelIndexFilter condition, int column)
{
	QModelIndex foundIndex;

	traverse(model, [&foundIndex, condition](const QModelIndex& idx) -> bool {
		if (condition(idx))
		{
			foundIndex = idx;
			return false;
		}
		return true;
	}, QModelIndex(), column);

	return foundIndex;
}


QStandardItem* napkin::findItemInModel(const QStandardItemModel& model, ModelItemFilter condition, int column)
{
	QStandardItem* foundItem = nullptr;

	traverse(model, [&foundItem, condition](QStandardItem* item) -> bool {
		if (condition(item))
		{
			foundItem = item;
			return false;
		}
		return true;
	}, QModelIndex(), column);

	return foundItem;
}

std::vector<rttr::type> napkin::getComponentTypes()
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

std::vector<rttr::type> napkin::getResourceTypes()
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

nap::rtti::ResolvedRTTIPath napkin::resolve(const nap::rtti::RTTIObject& obj, nap::rtti::RTTIPath path)
{
	nap::rtti::ResolvedRTTIPath resolvedPath;
	path.resolve(&obj, resolvedPath);
	assert(resolvedPath.isValid());
	return resolvedPath;
}

nap::rtti::RTTIObject* napkin::getPointee(const nap::rtti::RTTIObject& obj, const nap::rtti::RTTIPath& path)
{
	auto resolvedPath = resolve(obj, path);
	auto value = resolvedPath.getValue();
	auto value_type = value.get_type();
	auto wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;
	bool is_wrapper = wrapped_type != value_type;
	nap::rtti::RTTIObject* pointee = is_wrapper ? value.extract_wrapped_value().get_value<nap::rtti::RTTIObject*>()
												: value.get_value<nap::rtti::RTTIObject*>();
	return pointee;
}

bool napkin::setPointee(const nap::rtti::RTTIObject& obj, const nap::rtti::RTTIPath& path, const std::string& target)
{
	return false;
}

