#include "timelinemodel.h"

TLEvent::TLEvent(QObject* parent, const QString& name, const qreal start, const qreal end)
		: mName(name), mStart(start), mEnd(end),
		  QObject(parent) {
	mColor = QColor(Qt::cyan);
}

void TLEvent::setName(const QString& name) {
	mName = name;
	changed(*this);
}

void TLEvent::setStart(const qreal start) {
	mStart = start;
	changed(*this);
}

void TLEvent::setEnd(const qreal end) {
	mEnd = end;
	changed(*this);
}

void TLEvent::setColor(const QColor& col) {
	mColor = col;
	changed(*this);
}

void TLTrack::setName(const QString& name) {
	mName = name;
	changed(*this);
}

TLEvent* TLTrack::addEvent(const QString& name, qreal start, qreal end) {
	auto event = new TLEvent(this, name, start, end);
	mEvents << event;
	eventAdded(*event);
	return event;
}

TLTrack* TLTimeline::addTrack(const QString& name) {
	auto track = new TLTrack(this, name);
	mTracks << track;
	trackAdded(*track);
	return track;
}

void TLTimeline::removeTrack(TLTrack& track) {
	trackRemoved(track);
	mTracks.removeOne(&track);
}
