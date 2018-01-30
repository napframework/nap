#pragma once

#include <QWidget>
#include <QGraphicsView>
#include <QtWidgets/QSplitter>
#include "gridview.h"

namespace napkin {
	class CurveEditorScene : public QGraphicsScene
	{
	public:
	};

	class CurveEditorView : public GridView
	{
	public:
	};

	class CurveEditorPanel : public QWidget
	{
	public:
		CurveEditorPanel() : QWidget()
		{

		}

	private:
		QSplitter mSplitter;
	};

} // namespace napkin