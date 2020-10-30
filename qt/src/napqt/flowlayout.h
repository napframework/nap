/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QLayout>
#include <QStyle>


namespace nap
{
	namespace qt
	{
		/**
		 * Bsed on Qt's example: https://doc.qt.io/qt-5/qtwidgets-layouts-flowlayout-example.html
		 */
		class FlowLayout : public QLayout
		{
		public:
			explicit FlowLayout(QWidget* parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);
			explicit FlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
			~FlowLayout() override;

			void addItem(QLayoutItem* item) override;
			int horizontalSpacing() const;
			int verticalSpacing() const;
			Qt::Orientations expandingDirections() const override;
			bool hasHeightForWidth() const override;
			int heightForWidth(int) const override;
			int count() const override;
			QLayoutItem* itemAt(int index) const override;
			QSize minimumSize() const override;
			void setGeometry(const QRect& rect) override;
			QSize sizeHint() const override;
			QLayoutItem* takeAt(int index) override;

		private:
			int doLayout(const QRect& rect, bool testOnly) const;
			int smartSpacing(QStyle::PixelMetric pm) const;

			QList<QLayoutItem*> mItemList;
			int mHSpace;
			int mVSpace;
		};
	}
}

