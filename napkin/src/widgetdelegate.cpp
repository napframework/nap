#include "widgetdelegate.h"
#include "panels/inspectorpanel.h"
#include "typeconversion.h"
#include "appcontext.h"
#include <QtWidgets/QComboBox>
#include <generic/filterpopup.h>
#include <QtGui/QMouseEvent>

using namespace napkin;





void PropertyValueItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
									  const QModelIndex& index) const
{
	auto type = getTypeFromModelIndex(index);
	auto path = getPropertyPathFromIndex(index);
	const nap::rtti::TypeInfo wrapped_type = type.is_wrapper() ? type.get_wrapped_type() : type;

	// TODO: There must be a less convoluted way.
	// In the case of array elements, the type will be the array type, not the element type.
	// For now, grab the array's element type and use that.
	nap::rtti::TypeInfo wrapped_array_type = rttr::type::empty();
	if (type.is_array()) {
		nap::rtti::Variant value = path.getValue();
		nap::rtti::VariantArray array = value.create_array_view();
		nap::rtti::TypeInfo array_type = array.get_rank_type(array.get_rank());
		wrapped_array_type = array_type.is_wrapper() ? array_type.get_wrapped_type() : array_type;
	}

	if (type.is_enumeration())
	{

		uint val = index.model()->data(index, Qt::DisplayRole).toUInt();
		QStyleOptionViewItem op(option);

		op.text = enumIndexToQString(type.get_enumeration(), val);

		QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &op, painter);
	}
	else if (wrapped_type.is_pointer() || wrapped_array_type.is_pointer())
	{
		// Forward to draw text field
		QRect rect_txt = QRect(option.rect.left(),
							   option.rect.top(),
							   option.rect.width() - option.rect.height(),
							   option.rect.height());
		QRect rect_btn = QRect(option.rect.right() - option.rect.height(),
							   option.rect.top(),
							   option.rect.height(),
							   option.rect.height());

		QStyleOptionViewItem viewop(option);
		viewop.rect = rect_txt;
		QStyledItemDelegate::paint(painter, viewop, index);

		// Add pointer button
		QStyleOptionButton op;
		op.state = option.state;
		op.rect = rect_btn;
		op.text = "*";

		QApplication::style()->drawControl(QStyle::CE_PushButton, &op, painter);
	}
	else if (type == rttr::type::get<bool>())
	{

		QStyleOptionButton styleOption;
		styleOption.rect = option.rect;
		if (index.data(Qt::DisplayRole).toBool())
		{
			styleOption.state |= QStyle::State_On;
		}
		else
		{
			styleOption.state |= QStyle::State_Off;
		}
		QApplication::style()->drawControl(QStyle::CE_CheckBox, &styleOption, painter);
	}
	else
	{
		QStyledItemDelegate::paint(painter, option, index);
	}
}

QSize PropertyValueItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	return QStyledItemDelegate::sizeHint(option, index);
}

bool PropertyValueItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
											const QStyleOptionViewItem& option, const QModelIndex& index)
{

	auto type = getTypeFromModelIndex(index);
	if (type.is_enumeration())
	{
		return false;
	}
	else if (type == rttr::type::get<bool>())
	{
		if (event->type() == QEvent::MouseButtonPress)
		{
			bool val = index.data(Qt::DisplayRole).toBool();
			model->setData(index, !val, Qt::EditRole);
		}
		return true;
	}
	else
	{

		if (event->type() == QEvent::MouseButtonPress)
		{
			auto mouseEvent = dynamic_cast<QMouseEvent*>(event);
			QRect rect_btn = QRect(option.rect.right() - option.rect.height(),
								   option.rect.top(),
								   option.rect.right(),
								   option.rect.height());

			if (rect_btn.contains(mouseEvent->pos()))
			{

				auto propertyPath = getPropertyPathFromIndex(index);
				if (propertyPath.isValid() && propertyPath.getWrappedType().is_pointer()) {
					auto variant = index.data(Qt::UserRole);

					if (variant.canConvert<PropertyPath>()) {
						auto path = variant.value<PropertyPath>();
						auto selected = FilterPopup::getObject(AppContext::get().getQApplication()->activeWindow(),
															   path.getWrappedType());
						if (selected != nullptr)
							model->setData(index, QString::fromStdString(selected->mID), Qt::EditRole);
					}

				}
			}
		}
	}
	return QStyledItemDelegate::editorEvent(event, model, option, index);
}

rttr::type PropertyValueItemDelegate::getTypeFromModelIndex(const QModelIndex& index) const
{
	auto variant = index.data(Qt::UserRole);
	if (variant.canConvert<PropertyPath>())
	{
		return variant.value<PropertyPath>().getType();
	}
	return rttr::detail::get_invalid_type();
}

const PropertyPath PropertyValueItemDelegate::getPropertyPathFromIndex(const QModelIndex& idx) const
{
	auto variant = idx.data(Qt::UserRole);
	if (variant.canConvert<PropertyPath>())
		return variant.value<PropertyPath>();
	return PropertyPath();
}



QWidget* PropertyValueItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
												 const QModelIndex& index) const
{
	auto type = getTypeFromModelIndex(index);
	if (type.is_enumeration())
	{
		auto combo = new QComboBox(parent);
		combo->setFocusPolicy(Qt::StrongFocus);
		QStringList values;
		for (const auto& name : type.get_enumeration().get_names())
		{
			values << name.data();
		}
		combo->addItems(values);
		return combo;
	}
	return QStyledItemDelegate::createEditor(parent, option, index);
}

void PropertyValueItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	auto type = getTypeFromModelIndex(index);
	if (type.is_enumeration())
	{
		auto combo = dynamic_cast<QComboBox*>(editor);
		int value = index.data(Qt::EditRole).toInt();
		combo->setCurrentIndex(value);
	}
	else
	{
		return QStyledItemDelegate::setEditorData(editor, index);
	}
}

void PropertyValueItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	auto type = getTypeFromModelIndex(index);
	if (type.is_enumeration())
	{
		auto combo = dynamic_cast<QComboBox*>(editor);
		int value = combo->currentIndex();
		model->setData(index, value);
	}
	else
	{
		QStyledItemDelegate::setModelData(editor, model, index);
	}
}

