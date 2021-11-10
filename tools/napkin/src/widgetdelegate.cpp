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


PropertyValueItemDelegate::PropertyValueItemDelegate()
{
	mLinkIcon  = QIcon(QRC_ICONS_LINK);
	mFileIcon  = QIcon(QRC_ICONS_FILE);
	mColorIcon = QIcon(QRC_COLOR_WHEEL_FILE);
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
		QRect rect_txt = QRect(option.rect.left(), option.rect.top(), option.rect.width() - option.rect.height(), option.rect.height());

		int offset = int(float(option.rect.height()) * 0.25);
		QRect rect_btn = QRect
		(
			QPoint(option.rect.right() - option.rect.height() + offset, option.rect.top() + offset),
			QPoint(option.rect.right() - offset, option.rect.top() + option.rect.height() - offset)
		);

		// Text
		QStyleOptionViewItem viewop(option);
		viewop.rect = rect_txt;
		QStyledItemDelegate::paint(painter, viewop, index);

		// Get current color and set as background
		QString cur_color_str = index.model()->data(index, Qt::DisplayRole).toString();
		QColor background_color = getColorFromString(cur_color_str);
		if (!background_color.isValid())
		{
			background_color = Qt::white;
		}

		// Create and draw color picker icon
		painter->setBrush(QBrush(background_color));
		painter->setPen(QPen(option.palette.color(option.palette.Dark), 1.5f) );
		painter->setRenderHint(QPainter::Antialiasing);
		painter->drawEllipse(rect_btn);
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
		QRect rect_txt = QRect(option.rect.left(), option.rect.top(), option.rect.width() - option.rect.height(), option.rect.height());
		QRect rect_btn = QRect(option.rect.right() - option.rect.height(), option.rect.top(), option.rect.height(), option.rect.height()); 

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
			auto mouseEvent = dynamic_cast<QMouseEvent*>(event);
			QRect toggle_rect = QRect
			(
				QPoint(option.rect.left(), option.rect.top()),
				QPoint(option.rect.left() + option.rect.height(), option.rect.top() + option.rect.height())
			);

			if (toggle_rect.contains(mouseEvent->pos()))
			{
				bool val = index.data(Qt::DisplayRole).toBool();
				model->setData(index, !val, Qt::EditRole);
				return true;
			}
		}
		return false;
	}

	// Mouse click on icon
	if(event->type() == QEvent::MouseButtonPress)
	{
		QRect rect_btn = QRect(option.rect.right() - option.rect.height(), option.rect.top(), option.rect.right(), option.rect.height());
		auto mouseEvent = dynamic_cast<QMouseEvent*>(event);
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





