#pragma once

namespace nap
{
	namespace utility
	{
		template<class ITERATORTYPE, class ELEMENTTYPE>
		class UniquePtrMapIterator
		{
		public:
			UniquePtrMapIterator(ITERATORTYPE pos) :
				mPos(pos)
			{
			}

			UniquePtrMapIterator operator++()
			{
				mPos++;
				return *this;
			}

			bool operator!=(const UniquePtrMapIterator& rhs)
			{
				return rhs.mPos != mPos;
			}

			ELEMENTTYPE operator*() const
			{
				return mPos->second.get();
			}

			ITERATORTYPE mPos;
		};

		template<class MAPTYPE, class ELEMENTTYPE>
		class UniquePtrMapWrapper
		{
		public:
			UniquePtrMapWrapper(MAPTYPE& inMap) :
				mMap(&inMap)
			{
			}

			using Iterator = UniquePtrMapIterator<typename MAPTYPE::iterator, ELEMENTTYPE>;
			using ConstIterator = UniquePtrMapIterator<typename MAPTYPE::const_iterator, const ELEMENTTYPE>;

			Iterator begin() { return Iterator(mMap->begin()); }
			Iterator end() { return Iterator(mMap->end()); }
			ConstIterator begin() const { return ConstIterator(mMap->begin()); }
			ConstIterator end() const { return ConstIterator(mMap->end()); }

		private:
			MAPTYPE* mMap;
		};
	}
}
