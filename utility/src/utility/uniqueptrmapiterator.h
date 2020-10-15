/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

namespace nap
{
	namespace utility
	{
		/**
		 * Helper class that can be used to wrap an iterator to a map of unique_ptrs and extract its underlying type, without having to expose the unique_ptr itself.
		 */
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

		/**
		 * Helper class to wrap a map of unique_ptrs, allowing you to expose the map values to clients, while hiding the unique_ptr.
		 * This is the non-const version.
		 */
		template<class MAPTYPE, class ELEMENTTYPE>
		class UniquePtrMapWrapper
		{
		public:
			UniquePtrMapWrapper(MAPTYPE& inMap) :
				mMap(&inMap)
			{
			}

			using Iterator = UniquePtrMapIterator<typename MAPTYPE::iterator, ELEMENTTYPE>;

			Iterator begin() { return Iterator(mMap->begin()); }
			Iterator end() { return Iterator(mMap->end()); }

		private:
			MAPTYPE* mMap;
		};

		/**
		 * Helper class to wrap a map of unique_ptrs, allowing you to expose the map values to clients, while hiding the unique_ptr.
		 * This is the const version.
		 */
		template<class MAPTYPE, class ELEMENTTYPE>
		class UniquePtrConstMapWrapper
		{
		public:
			UniquePtrConstMapWrapper(const MAPTYPE& inMap) :
				mMap(&inMap)
			{
			}

			using ConstIterator = UniquePtrMapIterator<typename MAPTYPE::const_iterator, const ELEMENTTYPE>;

			ConstIterator begin() const { return ConstIterator(mMap->begin()); }
			ConstIterator end() const { return ConstIterator(mMap->end()); }

		private:
			const MAPTYPE* mMap;
		};
	}
}
