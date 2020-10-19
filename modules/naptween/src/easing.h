#pragma once

// external includes
#include <mathutils.h>

namespace nap
{
	enum Easing
	{
		LINEAR = 0,
		OUT_CUBIC = 1
	};

	template<typename T>
	class EaseBase
	{
	public:
		EaseBase() = default;
		virtual ~EaseBase() = default;

		virtual T evaluate(T& start, T& end, float progress) = 0;
	};

	template<typename T>
	class EaseLinear : public EaseBase<T>
	{
	public:
		EaseLinear() = default;
		virtual ~EaseLinear() = default;

		T evaluate(T& start, T& end, float progress) override;
	};

	//////////////////////////////////////////////////////////////////////////
	// explicit MSVC template specialization exports
	//////////////////////////////////////////////////////////////////////////
	template class NAPAPI EaseLinear<float>;

	//////////////////////////////////////////////////////////////////////////
	// template definitions
	//////////////////////////////////////////////////////////////////////////
	template<typename T>
	T EaseLinear<T>::evaluate(T& start, T& end, float progress)
	{
		return math::lerp<T>(start, end, progress);
	}
}