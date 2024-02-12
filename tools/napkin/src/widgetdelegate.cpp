/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "widgetdelegate.h"
#include "naputils.h"
#include "typeconversion.h"
#include "appcontext.h"
#include "napkin-resources.h"
#include "napkinutils.h"

#include <QComboBox>
#include <QMouseEvent>
#include <QFileDialog>
#include <QPainter>
#include <color.h>
#include <napqt/colorpicker.h>
#include <QCheckBox>

using namespace napkin;

/**
 * Converts a NAP hex display color (#RRGGBBAA) to QColor.
 * The color is invalid when conversion fails.
 */
static QColor getColorFromString(const QString& colorString)
{
	assert(colorString.startsWith('#'));
	assert((colorString.size() - 1) % 2 == 0);
	int channels = (colorString.size() - 1) / 2;

	// Compensate for alpha, this sucks but so does QT here: #AARRGGBB?
	// Colors are always ordered and displayed as #RRGGBBAA
	// Colors of only 1 value can't be converted to QColor, append 0 to form RGB
	QString rgb_string = colorString;
	uint alpha = 0xFF;

	switch (channels)
	{
	case 1:
	{
		rgb_string.append("0000");
		break;
	}
	case 3:
		break;
	case 4:
	{
		// Get alpha value
		bool valid = false;
		alpha = rgb_string.mid((3 * 2) + 1, 2).toUInt(&valid, 16);
		assert(valid);
		rgb_string.chop(2);
		break;
	}
	default:
		return QColor();
	}

	QColor r_col(rgb_string);
	if (r_col.isValid())
	{
		r_col.setAlpha(alpha);
		return r_col;
	}

	// Invalid
	return QColor();
}


void PropertyValueItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
									  const QModelIndex& index) const
{
	painter->save();
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
	else if (path.isNonEmbeddedPointer())
	{
		QStyleOptionViewItem viewop(option);
		QStyledItemDelegate::paint(painter, viewop, index);

		// Get btn rect
		QRect rect_btn = QRect(option.rect.right() - option.rect.height(),
			option.rect.top(),
			option.rect.height(),
			option.rect.height());

		// Add pointer button
		QIcon link_icon = AppContext::get().getResourceFactory().getIcon(QRC_ICONS_LINK);
		auto pixmap = link_icon.pixmap(rect_btn.size());
		painter->drawPixmap(rect_btn, pixmap, pixmap.rect());
	}
	else if (path.isColor())
	{
		// Text
		QStyleOptionViewItem viewop(option);
		QStyledItemDelegate::paint(painter, viewop, index);

		int offset = int(float(option.rect.height()) * 0.25);
		QRect rect_btn = QRect
		(
			QPoint(option.rect.right() - option.rect.height() + offset, option.rect.top() + offset),
			QPoint(option.rect.right() - offset, option.rect.top() + option.rect.height() - offset)
		);

		// Get current color and set as background
		QString cur_color_str = index.model()->data(index, Qt::DisplayRole).toString();
		QColor background_color = getColorFromString(cur_color_str);
		if (!background_color.isValid())
		{
			background_color = Qt::white;
		}

		// Create and draw color picker icon
		painter->setBrush(QBrush(background_color));
		painter->setPen(QPen(option.palette.color(option.palette.Text), 1.5f) );
		painter->setRenderHint(QPainter::Antialiasing);
		painter->drawEllipse(rect_btn);
	}
	else if (type == rttr::type::get<bool>())
	{
		// Draw regular (without text)
		QStyleOptionViewItem viewop(option);
		viewop.text.clear();
		QStyledItemDelegate::paint(painter, viewop, index);

		// Get icon size
		QRect rect_btn = QRect(option.rect.right() - option.rect.height() + 5,
			option.rect.top(),
			option.rect.height(),
			option.rect.height());

		// Draw checkbox
		QStyleOptionButton button_style;
		button_style.palette = option.palette;

		QColor frame_color = napkin::AppContext::get().getThemeManager().getColor(theme::color::dark1);
		QColor icon_color = napkin::AppContext::get().getThemeManager().getColor(theme::color::front4);
		button_style.palette.setBrush(QPalette::AlternateBase, frame_color);
		button_style.palette.setBrush(QPalette::Base, frame_color);
		button_style.palette.setBrush(QPalette::Highlight, frame_color);
		button_style.palette.setBrush(QPalette::Text, icon_color);

		button_style.rect = rect_btn;
		button_style.state |= index.data(Qt::DisplayRole).toBool() ? QStyle::State_On : QStyle::State_Off;
		QApplication::style()->drawControl(QStyle::CE_CheckBox, &button_style, painter);
	}
	else if (type == rttr::type::get<std::string>() && nap::rtti::hasFlag(path.getProperty(), nap::rtti::EPropertyMetaData::FileLink))
	{
		// Draw text
		QStyleOptionViewItem viewop(option);
		QStyledItemDelegate::paint(painter, viewop, index);

		// Get icon size
		QRect rect_btn = QRect(option.rect.right() - option.rect.height(),
			option.rect.top(),
			option.rect.height(),
			option.rect.height()); 

		// Add pointer button
		QIcon file_icon = AppContext::get().getResourceFactory().getIcon(QRC_ICONS_FILE);
		auto pixmap = file_icon.pixmap(rect_btn.size());
		painter->drawPixmap(rect_btn, pixmap, pixmap.rect());
	}
	else
	{
		QStyledItemDelegate::paint(painter, option, index);
	}
	painter->restore();
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
			auto mouseEvent = static_cast<QMouseEvent*>(event);

			// Get icon size
			QRect toggle_rect = QRect(
				option.rect.right() - option.rect.height() + 5,
				option.rect.top(),
				option.rect.height(),
				option.rect.height());

			if (toggle_rect.contains(mouseEvent->pos()))
			{
				bool val = index.data(Qt::DisplayRole).toBool();
				model->setData(index, !val, Qt::EditRole);
				return true;
			}
		}
	}

	// Mouse click on icon
	if(event->type() == QEvent::MouseButtonPress)
	{
		QRect rect_btn = QRect(option.rect.right() - option.rect.height(), option.rect.top(), option.rect.right(), option.rect.height());
		auto mouseEvent = static_cast<QMouseEvent*>(event);
		if (rect_btn.contains(mouseEvent->pos()))
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
					// Color as string
					QString cur_color_str = model->data(index, Qt::DisplayRole).toString();

					// Create color from string
					QColor current_qcolor = getColorFromString(cur_color_str);;
					if (!current_qcolor.isValid())
						current_qcolor = QColor(Qt::white);

					// Color picker dialog
					QColor color_sel = nap::qt::ColorPickerDialog::selectColor(AppContext::get().getMainWindow(), current_qcolor);
					if (color_sel.isValid())
					{
						// Convert to string, ie: #FF00FFFF
						std::string new_color = color_sel.name(QColor::HexRgb).toStdString();
						new_color += nap::utility::stringFormat("%02x", color_sel.alpha());

						// Set in model
						model->setData(index, QString::fromStdString(new_color), Qt::EditRole);
					}
					return true;
				}
			}

			if (type == rttr::type::get<std::string>() && nap::rtti::hasFlag(path.getProperty(), nap::rtti::EPropertyMetaData::FileLink))
			{
				auto& ctx = AppContext::get(); bool ok;
				std::string cur_path = path.getValue().to_string(&ok);
				QString dir = cur_path.empty() ? QString::fromStdString(ctx.getProjectInfo()->getDataDirectory()) :
					QFileInfo(getAbsoluteResourcePath(QString::fromStdString(cur_path))).path();

				auto parent = ctx.getMainWindow();
				auto filter = ctx.getResourceFactory().getFileFilter(path.getProperty());
				auto filename = napkin::utility::getOpenFilename(parent, "Select File", dir, filter);
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
		auto combo = qobject_cast<QComboBox*>(editor);
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
		auto combo = qobject_cast<QComboBox*>(editor);
		int value = combo->currentIndex();
		model->setData(index, value);
	}
	else
	{
		QStyledItemDelegate::setModelData(editor, model, index);
	}
}





