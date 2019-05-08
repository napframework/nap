#pragma once

#include <QtGui/QStandardItem>

namespace napkin
{

	/**
	 * An empty item.
	 */
	class EmptyItem : public QStandardItem
	{
	public:
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
		 * @param name Text to be displayed
		 */
		explicit InvalidItem(const QString& name);
	};

}; // napkin