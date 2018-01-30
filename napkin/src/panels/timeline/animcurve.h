#pragma once

#include <vector>
#include <algorithm>
#include <cassert>
#include <cmath>

template<typename T, typename V>
class AnimCurve {
public:
	enum Interp {
		Linear, Bezier
	};

	enum Infinity {
		Hold, Loop, Bounce
	};

	AnimCurve() = default;

	size_t getKeyCount() const;

	const Infinity& getPosInfinity() const { return mPosInfinity; }
	void setPosInfinity(const Infinity& inf) { mPosInfinity = inf; }

	const Infinity& getNegInfinity() const { return mNegInfinity; }
	void setNegInfinity(const Infinity& inf) { mNegInfinity = inf; }

	void addKey(const T& time, const V& value,
				const T& inTanTime, const V& inTanValue,
				const T& outTanTime, const V& outTanValue,
				const Interp& interp, bool tanLocked);

	void removeKey(size_t index);

	V evaluate(const T& time, const V& def) const;


private:
	template<typename T, typename V>
	struct Key {
		Key(const T& time, const V& value,
			const T& inTanTime, const V& inTanValue,
			const T& outTanTime, const V& outTanValue,
			const Interp& interp, bool tanLocked);

		T mTime;
		V mValue;
		T mInTanTime;
		V minTanValue;
		T mOutTanTime;
		V mOutTanValue;
		Interp mInterp;
		bool mTanLocked;
	};

	V bezier(const V& p[4], const T& t) const;
	T tForXBezier(const T&* pts, const T& x, const T& threshold = 0.01, int maxiterations = 10) const;

	V evalSegment(const Key& k0, const Key& k1, const T& time) const;
	void  wrapInfinity(const T& beginTime, const T& endTime, const Infinity& inf, T& time) const;
	size_t findSegment(const T& time) const;

	void loop(T& t, const T& low, const T& high) const;
	void bounce(T& t, const T& low, const T& high) const;

	Infinity mNegInfinity = Hold;
	Infinity mPosInfinity = Hold;
	std::vector<Key<T, V>> mKeys;
};



template<typename T, typename V>
AnimCurve<T, V>::Key<T, V>::Key(const T& time, const V& value,
								const T& inTanTime, const V& inTanValue,
								const T& outTanTime, const V& outTanValue,
								const Interp& interp, const bool tanLocked)
		: mTime(time), mValue(value),
		  mInTanTime(inTanTime), minTanValue(inTanValue),
		  mOutTanTime(outTanTime), mOutTanValue(outTanValue),
		  mInterp(interp), mTanLocked(tanLocked) {
}

template<typename T, typename V>
size_t AnimCurve<T, V>::getKeyCount() const { return mKeys.size(); }


template<typename T, typename V>
void
AnimCurve<T, V>::addKey(const T& time, const V& value,
						const T& inTanTime, const V& inTanValue,
						const T& outTanTime, const V& outTanValue,
						const AnimCurve::Interp& interp, const bool tanLocked)
{
	mKeys.emplace_back(time, value, inTanTime, inTanValue, outTanTime, outTanValue, interp, tanLocked);
}


template<typename T, typename V>
void AnimCurve<T, V>::removeKey(size_t index)
{
	mKeys.erase(mKeys.begin() + index);
}

template<typename T, typename V>
V AnimCurve<T, V>::bezier(const V& p[4], const T& t) const
{
	const V u = 1 - t;
	const V a = u * u * u;
	const V b = 3 * u * u * t;
	const V c = 3 * u * t * t;
	const V d = t * t * t;
	return p[0] * a + p[1] * b + p[2] * c + p[3] * d;
}

template<typename T, typename V>
V AnimCurve<T, V>::evaluate(const T& time, const V& def) const
{
	T t = time;
	// No keys, return provided default
	if (mKeys.empty())
		return def;

	// One key, return its value
	if (mKeys.size() == 1)
		return mKeys[0].mValue;

	// Wrap time to handle infinity
	const T& beginTime = mKeys[0].mTime;
	const T& endTime = mKeys[mKeys.size()-1].mTime;
	if (time <= beginTime)
		wrapInfinity(beginTime, endTime, mNegInfinity, time);
	else if (time >= endTime)
		wrapInfinity(beginTime, endTime, mPosInfinity, time);

	// We only need to evaluate one segment
	size_t seg = findSegment(t);

	// Do evaluation
	return evalSegment(mKeys[seg], mKeys[seg + 1], t);
}

template<typename T, typename V>
void AnimCurve<T, V>::wrapInfinity(const T& beginTime, const T& endTime, const AnimCurve::Infinity& inf, T& time) const
{
	switch (inf) {
		case Hold:
			time = beginTime;
		case Loop:
			loop(time, beginTime, endTime);
			break;
		case Bounce:
			bounce(time, beginTime, endTime);
			break;
	}
}



template<typename T, typename V>
size_t AnimCurve<T, V>::findSegment(const T& time) const
{
	const size_t len = mKeys.size()-1;
	for (size_t i=1; i < len; i++)
		if (time < mKeys[i].mTime)
			return i;

	return len;
}

template<typename T, typename V>
V AnimCurve<T, V>::evalSegment(const Key& a, const Key& b, const T& time) const {
	const T ptsT[] = {a.mTime,
					  a.mTime + a.mOutTanTime,
					  b.mTime + b.mInTanTime,
					  b.mTime};
	const V ptsV[] = {a.mValue,
					  a.mValue + a.mOutTanValue,
					  b.mValue + b.minTanValue,
					  b.mValue};



	if (a.mInterp == Linear) {
		T at = ptsT[0];
		V av = ptsV[0];
		T bt = ptsT[3];
		V bv = ptsV[3];
		T dt = bt - at;
		T t = (t-at) / dt;
		T pt = at + dt * t;
		V pv = av + (bv-av) * t;
		return pv;
	} else if (a.mInterp == Bezier) {
		const T t = tForXBezier(ptsT, time);
		return bezier(ptsV, t);
	}
	assert(false);
}

template<typename T, typename V>
T AnimCurve<T, V>::tForXBezier(const T&* pts, const T& x, const T& threshold, int maxiterations) const {
	T depth = 0.5;
	T t = 0.5;
	for (int i=0; i<maxiterations; i++)
	{
		T dt = x - bezier(pts, t);
		if (fabs(dt) <= threshold)
			break;

		depth *= 0.5;
		t += dt > 0 ? depth : -depth;
	}
	return t;
}

template<typename T, typename V>
void AnimCurve<T, V>::loop(T& t, const T& low, const T& high) const {
	t = low + fmod(t-low, high-low);
}

template<typename T, typename V>
void AnimCurve<T, V>::bounce(T& t, const T& low, const T& high) const {
	T d = high-low;
	t = fabs(fmod(((t-1.0)/d)-1.0, 2.0)) * d + 1;
}


