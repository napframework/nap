#pragma once


#include <QGraphicsRectItem>
#include "timelinemodel.h"

namespace napkin {

	class EventItem : public QGraphicsRectItem {
	public:
		explicit EventItem(Event& event) : mClip(event), QGraphicsRectItem() {}

		Event& event() const { return mClip; }

	private:
		Event& mClip;
	};
}