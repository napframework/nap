#include "curveview.h"

using namespace napqt;

#define DEFAULT_SCENE_EXTENT 1000

CurveScene::CurveScene()
{
	setSceneRect(-DEFAULT_SCENE_EXTENT, -DEFAULT_SCENE_EXTENT,
			DEFAULT_SCENE_EXTENT * 2, DEFAULT_SCENE_EXTENT * 2);
}
CurveView::CurveView(QWidget* parent) : GridView(parent)
{
	setScene(&mCurveScene);
}
