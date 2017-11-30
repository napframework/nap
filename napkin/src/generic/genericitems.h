#pragma once

#include <QtGui/QStandardItem>

namespace napkin
{
	enum GenericStandardItemTypeID
	{
		EmptyItemTypeID = 10,
		InvalidItemTypeID = 11,
	};


	/**
	 * An empty item.
	 */
	class EmptyItem : public QStandardItem
	{
	public:
		/**
		 * QStandardItem is not a QObject, so regular QObject polymorphism doesn't work.
		 * This function is supposed to solve that.
		 * See: http://doc.qt.io/qt-5/qstandarditem.html#type
		 */
		int type() const override;

		/**
		 * Constructor
		 */
		EmptyItem();
	};

	/**
	 * An item representing invalid data
	 */
	class InvalidItem : public QStandardItem
	{
	public:
		/**
		 * QStandardItem is not a QObject, so regular QObject polymorphism doesn't work.
		 * This function is supposed to solve that.
		 * See: http://doc.qt.io/qt-5/qstandarditem.html#type
		 */
		int type() const override;

		/**
		 * @param name Text to be displayed
		 */
		explicit InvalidItem(const QString& name);
	};

}; // napkin