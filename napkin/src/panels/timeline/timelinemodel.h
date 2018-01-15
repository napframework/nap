#pragma  once

#include <QString>
#include <QObject>
#include <QtGui/QColor>

namespace napkin {

	class Timeline;
	class Track;

	class Event : public QObject {
	Q_OBJECT
	public:
		Event(Track& parent, const QString& name, qreal start, qreal end);

		const QString& name() const { return mName; }

		void setName(const QString& name);

		qreal start() const { return mStart; }

		void setStart(qreal start);

		qreal end() const { return mEnd; }

		void setEnd(qreal end);

		qreal length() const;

		const QColor& color() const { return mColor; }

		void setColor(const QColor& col);

		Track& track() const;

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
		Track(Timeline& parent, const QString& name);

		int height() const { return mHeight; }

		const QString& name() const { return mName; }

		void setName(const QString& name);

		Event* addEvent(const QString& name, qreal start, qreal end);

		const QList<Event*>& events() const { return mEvents; }

		Timeline& timeline() const;

		int index();

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