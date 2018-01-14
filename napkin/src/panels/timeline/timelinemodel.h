#pragma  once

#include <QString>
#include <QObject>
#include <QtGui/QColor>

class TLEvent : public QObject {
Q_OBJECT
public:
	TLEvent(QObject* parent, const QString& name, const qreal start, const qreal end);
	const QString& name() const { return mName; }
	void setName(const QString& name);
	qreal start() const { return mStart; }
	void setStart(const qreal start);
	qreal end() const { return mEnd; }
	void setEnd(const qreal end);
	const QColor& color() const { return mColor; }
	void setColor(const QColor& col);
Q_SIGNALS:
	void changed(TLEvent& event);

private:
	qreal mStart;
	qreal mEnd;
	QString mName;
	QColor mColor;
};

class TLTrack : public QObject {
Q_OBJECT
public:
	TLTrack(QObject* parent, const QString& name) : mName(name), QObject(parent) {}
	int height() const { return mHeight; }
	const QString& name() const { return mName; }
	void setName(const QString& name);
	TLEvent* addEvent(const QString& name, qreal start, qreal end);
	const QList<TLEvent*>& events() const { return mEvents; }

Q_SIGNALS:
	void eventAdded(TLEvent& event);
	void eventRemoved(TLEvent& event);
	void changed(TLTrack& track);


private:
	QString mName;
	QList<TLEvent*> mEvents;
	int mHeight = 30;
};


class TLTimeline : public QObject {
Q_OBJECT
public:
	TLTimeline(QObject* parent) : QObject(parent) {}
	TLTrack* addTrack(const QString& name);
	void removeTrack(TLTrack& track);
	const QList<TLTrack*>& tracks() const { return mTracks; }

Q_SIGNALS:
	void trackAdded(TLTrack& track);
	void trackRemoved(TLTrack& track);

private:
	QList<TLTrack*> mTracks;

};