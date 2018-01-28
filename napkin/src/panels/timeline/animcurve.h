#pragma once

#include <vector>
#include <algorithm>
#include <cassert>

template<typename T, typename V>
class AnimCurve {
public:
	enum Interp {
		Linear, Bezier
	};

	AnimCurve() = default;

	size_t pointCount() const;;

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

	T tForX(const T& x, const T& threshold=0.001, int maxiterations=10) const;

	V evalSegment(const Key& k0, const Key& k1, const T& time) const;

	size_t findSegment(const T& time) const;


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
size_t AnimCurve<T, V>::pointCount() const { return mKeys.size(); }


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
	return p0 * a + p1 * b + p2 * c + p3 * d;
}

template<typename T, typename V>
V AnimCurve<T, V>::evaluate(const T& time, const V& def) const
{
	// No keys, return provided default
	if (mKeys.empty())
		return def;

	// One key, return its value
	if (mKeys.size() == 1)
		return mKeys[0].mValue;

	// First key or earlier
	// TODO: Implement infinity
	if (time <= mKeys[0].mTime)
		return mKeys[0].mValue;

	Key& k0 = mKeys[mKeys.size()-1];

	// Last key or later
	// TODO: Implement infinity
	if (time >= k0.mTime)
		return k0.mValue;

	size_t seg = findSegment(time);

	return evalSegment(mKeys[seg], mKeys[seg+1], time);
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
V AnimCurve<T, V>::evalSegment(const AnimCurve::Key& a, const AnimCurve::Key& b, const T& time) const {
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
		const T t = tForX(ptsT, time);
		return bezier(ptsV, t);
	}
	assert(false);
}

