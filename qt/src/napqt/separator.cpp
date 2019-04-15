#include "separator.h"

nap::qt::Separator::Separator(Qt::Orientation orientation) : QFrame()
{
	if (orientation == Qt::Horizontal)
		setFrameShape(QFrame::HLine);
	else if (orientation == Qt::Vertical)
		setFrameShape(QFrame::VLine);
	setFrameShadow(QFrame::Sunken);
}
