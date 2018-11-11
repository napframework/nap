#pragma once

#include <QWidget>
#include <QSplitter>
#include <QItemDelegate>
#include <napqt/filtertreeview.h>

#include <napqt/qtutils.h>
#include "curveview.h"
#include "curvetreemodel.h"

namespace napqt
{

	class CurveTreeIconDelegate
	{
	public:
		void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
			painter->fillRect(option.rect, Qt::red);
		}

	};

	class CurveTreeDelegate : public QItemDelegate
	{
	public:
		void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
		QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

		void addDelegate(std::shared_ptr<CurveTreeIconDelegate> delegate);
	private:
		QList<std::shared_ptr<CurveTreeIconDelegate>> mIconDelegates;
		int mIconSpacing = 4;
		QMargins mIconMargins = {8, 2, 16, 2};
	};

	class CurvePanel : public QWidget
	{
	Q_OBJECT
	public:
		CurvePanel(QWidget* parent = nullptr);

		void setModel(AbstractCurveModel* model);


	private:
		void onTreeDoubleClicked(const QModelIndex& idx);

		QVBoxLayout mLayout;
		QSplitter mSplitter;
		CurveView mCurveView;
		FilterTreeView mTreeView;
		CurveTreeModel mTreeModel;
		CurveTreeDelegate mDelegate;
	};
}