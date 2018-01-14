#include "timelineview.h"

#include <QtGui/QKeyEvent>
#include <QWidget>

using namespace napkin;



TimelineView::TimelineView()
		: QGraphicsView() {
	setFocusPolicy(Qt::FocusPolicy::ClickFocus);
	setMouseTracking(true);

}
