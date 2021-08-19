/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "widgetdelegate.h"
#include "naputils.h"
#include "typeconversion.h"
#include "appcontext.h"
#include "napkin-resources.h"

#include <QComboBox>
#include <QMouseEvent>
#include <QFileDialog>
#include <QPainter>
#include <color.h>
#include <napqt/colorpicker.h>

using namespace napkin;

PropertyValueItemDelegate::PropertyValueItemDelegate()
{
	mLinkIcon  = QIcon(QRC_ICONS_LINK);
	mFileIcon  = QIcon(QRC_ICONS_FILE);
	mColorIcon = QIcon(QRC_COLOR_WHEEL_FILE);
}


void PropertyValueItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
									  const QModelIndex& index) const
{
	QVariant col = index.data(Qt::BackgroundRole);
	if (col.isValid())
	{
		painter->fillRect(option.rect, col.value<QColor>());
	}

	auto type = getTypeFromModelIndex(index);
	auto path = getPropertyPathFromIndex(index);

	if (path.isEnum())
	{
		uint val = index.model()->data(index, Qt::DisplayRole).toUInt();
		QStyleOptionViewItem op(option);
		op.text = enumIndexToQString(type.get_enumeration(), val);
		QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &op, painter);
	}
	else if (path.isPointer())
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
		auto pixmap = mLinkIcon.pixmap(rect_btn.size());
		painter->drawPixmap(rect_btn, pixmap, pixmap.rect());
	}
	else if (path.isColor())
	{
		// Forward to draw text field
		QRect rect_txt = QRect( option.rect.left(),  option.rect.top(), option.rect.width() - option.rect.height(), option.rect.height());
		QRect rect_btn = QRect(option.rect.right() - option.rect.height(), option.rect.top(), option.rect.height(), option.rect.height());
		QStyleOptionViewItem viewop(option);
		viewop.rect = rect_txt;
		QStyledItemDelegate::paint(painter, viewop, index);

		// Add pointer button
		auto pixmap = mColorIcon.pixmap(rect_btn.size());
		painter->drawPixmap(rect_btn, pixmap, pixmap.rect());
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
	else if (type == rttr::type::get<std::string>() && nap::rtti::hasFlag(path.getProperty(), nap::rtti::EPropertyMetaData::FileLink))
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
		auto pixmap = mFileIcon.pixmap(rect_btn.size());
		painter->drawPixmap(rect_btn, pixmap, pixmap.rect());
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


bool PropertyValueItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
	auto path = getPropertyPathFromIndex(index);
	auto type = getTypeFromModelIndex(index);

	// Enum
	if (path.isEnum())
	{
		return false;
	}

	// Toggle
	if (type == rttr::type::get<bool>())
	{
		if (event->type() == QEvent::MouseButtonPress)
		{
			bool val = index.data(Qt::DisplayRole).toBool();
			model->setData(index, !val, Qt::EditRole);
		}
		return true;
	}

	// Mouse click on icon
	QRect rect_btn = QRect(option.rect.right() - option.rect.height(), option.rect.top(), option.rect.right(), option.rect.height());
	auto mouseEvent = dynamic_cast<QMouseEvent*>(event);
	if(event->type() == QEvent::MouseButtonPress && rect_btn.contains(mouseEvent->pos()))
	{

		// TODO: There must be a less convoluted way.
		// In the case of array elements, the type will be the array type, not the element type.
		// For now, grab the array's element type and use that.
		nap::rtti::TypeInfo wrapped_type = path.getWrappedType();
		if (type.is_array())
		{
			nap::rtti::Variant value = path.getValue();
			nap::rtti::VariantArray array = value.create_array_view();
			nap::rtti::TypeInfo array_type = array.get_rank_type(array.get_rank());
			wrapped_type = array_type.is_wrapper() ? array_type.get_wrapped_type() : array_type;
		}

		if (path.isPointer())
		{
			auto variant = index.data(Qt::UserRole);

			if (variant.canConvert<PropertyPath>())
			{
				auto path = variant.value<PropertyPath>();
				auto objects = AppContext::get().getDocument()->getObjects(wrapped_type);
				auto selected = napkin::showObjectSelector(AppContext::get().getMainWindow(), objects);
				if (selected != nullptr)
					model->setData(index, QString::fromStdString(selected->mID), Qt::EditRole);

				return true;
			}

		}

		if (path.isColor())
		{
			auto variant = index.data(Qt::UserRole);
			if (variant.canConvert<PropertyPath>())
			{
				QString cur_color_str  = model->data(index, Qt::DisplayRole).toString();
				assert(cur_color_str.startsWith('#'));
				assert((cur_color_str.size() - 1) % 2 == 0);

				// Compensate for alpha, this sucks but so does QT #AARRGGBB!
				// NAP colors are always ordered and displayed as #RRGGBBAA
				if ((cur_color_str.size() -1) / 2 == 4)
				{
					QString red = cur_color_str.mid(1, 2);
					cur_color_str.replace((0*2) + 1, 2, cur_color_str.mid(3 * 2, 2));
					cur_color_str.replace((3*2) + 1, 2, red);
				}
				std::string temp_str = cur_color_str.toStdString();

				// Create color
				QColor current_qcolor(cur_color_str);
				if (!current_qcolor.isValid())
				{
					assert(false);
					current_qcolor = QColor(0xFF, 0xFF, 0xFF, 0xFF);
				}

				// Color picker dialog
				nap::qt::ColorPickerDialog dialog(AppContext::get().getMainWindow(), current_qcolor);
				dialog.move(QCursor::pos());
				dialog.setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
				dialog.exec();

				// Get selected color
				QColor selected_color = dialog.getColor();

				// Convert to string, ie: #FF00FFFF
				std::string new_color = selected_color.name(QColor::HexRgb).toStdString();
				new_color += nap::utility::stringFormat("%02x", selected_color.alpha());

				// Set in model
				model->setData(index, QString::fromStdString(new_color), Qt::EditRole);

				return true;
			}
		}

		if (type == rttr::type::get<std::string>() && nap::rtti::hasFlag(path.getProperty(), nap::rtti::EPropertyMetaData::FileLink))
		{
			bool ok;

			QString currentFilePath = getAbsoluteResourcePath(QString::fromStdString(path.getValue().to_string(&ok)));
			QString dir = QFileInfo(currentFilePath).path();

			auto& ctx = AppContext::get();
			auto parent = ctx.getMainWindow();
			auto filter = ctx.getResourceFactory().getFileFilter(path.getProperty());
			auto filename = QFileDialog::getOpenFileName(parent, "Select File", dir, filter, &filter);
			if (!filename.isEmpty())
			{
				// Make relative if inside resource dir
				if (nap::qt::directoryContains(getResourceReferencePath(), filename))
					filename = getRelativeResourcePath(filename);
				model->setData(index, filename);
			}
			return true;
		}
	}

	// Default
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
	{
		return variant.value<PropertyPath>();
	}
	return {};
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





