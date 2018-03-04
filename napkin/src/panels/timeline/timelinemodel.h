#pragma  once

#include <QString>
#include <QObject>
#include <QtGui/QColor>

namespace napkin {




	class Range
	{
	public:
		Range() : mStart(0), mEnd(0) {}
		Range(qreal start, qreal end) : mStart(start), mEnd(end) {}
		Range(const Range& other) : mStart(other.start()), mEnd(other.end()) {}
		qreal start() const { return mStart; }
		void setStart(qreal start) { mStart = start; }
		qreal end() const { return mEnd; }
		void setEnd(qreal end) { mEnd = end; }
		qreal length() const { return mEnd - mStart; }
		void set(qreal start, qreal end) { mStart = start; mEnd = end; }
		void set(const Range& range) { mStart = range.start(); mEnd = range.end(); }
		bool operator==(const Range& other) const { return mStart == other.start() && mEnd == other.end(); }
		operator const QString() const { return QString("Range(%1, %2)").arg(QString::number(mStart), QString::number(mEnd)); }
	private:
		qreal mStart;
		qreal mEnd;
	};

	class Timeline;
	class Track;

	class Event : public QObject {
	Q_OBJECT
	public:
		Event(Track& parent, const QString& name, qreal start, qreal end);

		const QString& name() const { return mName; }

		void setName(const QString& name);

		qreal start() const { return mRange.start(); }

		void setStart(qreal start);

		qreal end() const { return mRange.end(); }

		void setEnd(qreal end);

		qreal length() const;

		const QColor& color() const { return mColor; }

		void setColor(const QColor& col);

		Track& track() const;

		void setTrack(Track& track);

	Q_SIGNALS:

		void changed(Event& event);

	private:
		Range mRange;
		QString mName;
		QColor mColor;
	};

	class Track : public QObject {
	Q_OBJECT
	public:
		Track(QObject& parent, const QString& name);

		int height() const { return mHeight; }

		const QString& name() const { return mName; }

		void setName(const QString& name);

		Event* addEvent(const QString& name, qreal start, qreal end);

		Track* addTrack(const QString& name);

		const QList<Event*>& events() const { return mEvents; }

		Timeline& timeline() const;

		const QList<Track*>& childTracks() const { return mChildren; }

		int index();

	Q_SIGNALS:
		void trackAdded(Track& track);
		void trackRemoved(Track& track);

		void eventAdded(Event& event);
		void eventRemoved(Event& event);
		void changed(Track& track);


	private:
		QString mName;
		QList<Event*> mEvents;
		QList<Track*> mChildren;
		int mHeight = 30;
	};


	class Timeline : public QObject {
	Q_OBJECT
	public:
		Timeline(QObject* parent) : QObject(parent) {}

		Track* addTrack(const QString& name, Track* parent = nullptr);

		void removeTrack(Track& track);

		const QList<Track*>& tracks() const { return mTracks; }

		int framerate() const { return mFramerate; }

	Q_SIGNALS:
		void trackAdded(Track& track);
		void trackRemoved(Track& track);
		void eventAdded(Event& event);
		void eventRemoved(Event& event);

	private:
		QList<Track*> mTracks;
		int mFramerate = 30;
	};

}