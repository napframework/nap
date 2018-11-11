#include "curvepanel.h"

using namespace napkin;
using namespace napqt;

CurvePanel::CurvePanel(QWidget* parent) : QWidget(parent)
{
	mLayout.setContentsMargins(0, 0, 0, 0);
	setLayout(&mLayout);
	mLayout.addWidget(&mCurveView);
}
