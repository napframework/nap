#pragma once

#include <QGraphicsScene>
#include <napqt/gridview.h>


namespace napqt
{


	class CurveItem : public QGraphicsPathItem
	{

	};

	class CurveScene : public QGraphicsScene
	{
	public:
		CurveScene();
	private:
	};

	class CurveView : public GridView
	{
	Q_OBJECT
	public:
		explicit CurveView(QWidget* parent=nullptr);

	private:
		CurveScene mCurveScene;
	};

}
