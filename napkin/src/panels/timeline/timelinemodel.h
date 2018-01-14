#pragma  once

#include <QString>
#include <QObject>
#include <QtGui/QColor>

namespace napkin {

	class Event : public QObject {
	Q_OBJECT
	public:
		Event(QObject* parent, const QString& name, const qreal start, const qreal end);

		const QString& name() const { return mName; }

		void setName(const QString& name);

		qreal start() const { return mStart; }

		void setStart(const qreal start);

		qreal end() const { return mEnd; }

		void setEnd(const qreal end);

		const QColor& color() const { return mColor; }

		void setColor(const QColor& col);

	Q_SIGNALS:

		void changed(Event& event);

	private:
		qreal mStart;
		qreal mEnd;
		QString mName;
		QColor mColor;
	};

	class Track : public QObject {
	Q_OBJECT
	public:
		Track(QObject* parent, const QString& name) : mName(name), QObject(parent) {}

		int height() const { return mHeight; }

		const QString& name() const { return mName; }

		void setName(const QString& name);

		Event* addEvent(const QString& name, qreal start, qreal end);

		const QList<Event*>& events() const { return mEvents; }

	Q_SIGNALS:

		void eventAdded(Event& event);

		void eventRemoved(Event& event);

		void changed(Track& track);


	private:
		QString mName;
		QList<Event*> mEvents;
		int mHeight = 30;
	};


	class Timeline : public QObject {
	Q_OBJECT
	public:
		Timeline(QObject* parent) : QObject(parent) {}

		Track* addTrack(const QString& name);

		void removeTrack(Track& track);

		const QList<Track*>& tracks() const { return mTracks; }

	Q_SIGNALS:

		void trackAdded(Track& track);

		void trackRemoved(Track& track);

	private:
		QList<Track*> mTracks;

	};

}