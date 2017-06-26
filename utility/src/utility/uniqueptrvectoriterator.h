#pragma once

namespace nap
{
	namespace utility
	{
		template<class ITERATORTYPE, class ELEMENTTYPE>
		class UniquePtrVectorIterator
		{
		public:
			UniquePtrVectorIterator(ITERATORTYPE pos) :
				mPos(pos)
			{
			}

			UniquePtrVectorIterator operator++()
			{
				mPos++;
				return *this;
			}

			bool operator!=(const UniquePtrVectorIterator& rhs)
			{
				return rhs.mPos != mPos;
			}

			ELEMENTTYPE operator*() const
			{
				return mPos->get();
			}

			ITERATORTYPE mPos;
		};

		template<class VECTORTYPE, class ELEMENTTYPE>
		class UniquePtrVectorWrapper
		{
		public:
			UniquePtrVectorWrapper(VECTORTYPE& inVector) :
				mVector(&inVector)
			{
			}

			using Iterator = UniquePtrVectorIterator<typename VECTORTYPE::iterator, ELEMENTTYPE>;
			using ConstIterator = UniquePtrVectorIterator<typename VECTORTYPE::const_iterator, const ELEMENTTYPE>;

			Iterator begin() { return Iterator(mVector->begin()); }
			Iterator end() { return Iterator(mVector->end()); }
			ConstIterator begin() const { return ConstIterator(mVector->begin()); }
			ConstIterator end() const { return ConstIterator(mVector->end()); }

		private:
			VECTORTYPE* mVector;
		};
	}
}