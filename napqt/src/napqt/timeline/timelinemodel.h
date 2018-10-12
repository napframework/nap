#pragma  once

#include <QString>
#include <QObject>
#include <QColor>

namespace napkin {




	class Range
	{
	public:
		Range() : mStart(0), mEnd(0) {}
		Range(qreal start, qreal end) : mStart(start), mEnd(end) {}
		Range(const Range& other) : mStart(other.start()), mEnd(other.end()) {}
		qreal start() const { return mStart; }
		void setStart(qreal start) { mStart = start; }
		void move(qreal delta) { mStart += delta; mEnd += delta; }
		void moveTo(qreal start) { qreal length = mEnd - mStart; mStart = start; mEnd = mStart + length; }
		qreal end() const { return mEnd; }
		void setEnd(qreal end) { mEnd = end; }
		qreal length() const { return mEnd - mStart; }
		void set(qreal start, qreal end) { mStart = start; mEnd = end; }
		void set(const Range& range) { mStart = range.start(); mEnd = range.end(); }
		bool operator==(const Range& other) const { return mStart == other.start() && mEnd == other.end(); }
		Range operator+(qreal off) const { return offset(off); }
		operator const QString() const { return QString("Range(%1, %2)").arg(QString::number(mStart), QString::number(mEnd)); }
		Range offset(qreal delta) const { Range r(*this); r.move(delta); return r; }
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
		void moveTo(qreal start);
		void move(qreal delta);
		void setRange(const Range& range);
		qreal end() const { return mRange.end(); }
		void setEnd(qreal end);
		qreal length() const;
		const QColor& color() const { return mColor; }
		void setColor(const QColor& col);
		Track& track() const;
		void setTrack(Track& track);
		qreal minLength() const;
		qreal maxLength() const;
	Q_SIGNALS:

		void changed(Event& event);

	private:
		Range mRange;
		QString mName;
		QColor mColor;
	};

	class Tick : public QObject {
		Q_OBJECT
	public:
		Tick(Track& parent, qreal time);
		qreal time() const { return mTime; }
		void setTime(qreal time);
		Track& track() { return *(Track*) parent(); }
		const QColor color() const { return Qt::red; }
	Q_SIGNALS:
		void changed(Tick& tick);
	private:
		qreal mTime;
	};

	class Track : public QObject {
	Q_OBJECT
	public:
		Track(QObject& parent, const QString& name);
		void setHeight(int height) { mHeight = height; changed(*this); }
		int height() const { return mHeight; }
		const QString& name() const { return mName; }
		void setName(const QString& name);
		Event* addEvent(const QString& name, qreal start, qreal end);
		Tick* addTick(qreal time);
		Track* addTrack(const QString& name);
		const QList<Event*>& events() const { return mEvents; }
		const QList<Tick*>& ticks() const { return mTicks; }
		void eventsRecursive(QList<Event*>& events) const;
		Timeline& timeline() const;
		const QList<Track*>& childTracks() const { return mChildren; }
		Track* parentTrack() const;
		int index();
		bool range(qreal& start, qreal& end);

	Q_SIGNALS:
		void trackAdded(Track& track);
		void trackRemoved(Track& track);

		void eventAdded(Event& event);
		void tickAdded(Tick& tick);
		void eventRemoved(Event& event);
		void tickRemoved(Tick& tick);
		void changed(Track& track);


	private:
		QString mName;
		QList<Event*> mEvents;
		QList<Tick*> mTicks;
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

		qreal minEventLength() const { return mMinEventLength; }
		void setMinEventLength(qreal len) { mMinEventLength = len; }
		qreal maxEventLength() const { return mMaxEventLength; }
		void setMaxEventLength(qreal len) { mMaxEventLength = len; }

	Q_SIGNALS:
		void trackAdded(Track& track);
		void trackRemoved(Track& track);
		void eventAdded(Event& event);
		void eventRemoved(Event& event);


	private:
		QList<Track*> mTracks;
		int mFramerate = 30;
		qreal mMinEventLength = 1;
		qreal mMaxEventLength = -1;
	};
}