/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

namespace nap
{
	namespace utility
	{
		/**
		 * Helper class that can be used to wrap an iterator to a vector of unique_ptrs and extract its underlying type, 
		  without having to expose the unique_ptr itself.
		 */
		template<class ITERATORTYPE, class ELEMENTTYPE>
		class UniquePtrVectorIterator
		{
		public:
			UniquePtrVectorIterator(ITERATORTYPE pos) :
				mPos(pos)
			{ }

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


		/**
		 * Helper class to wrap a vector of unique_ptrs, allowing you to expose the vector to clients, 
		 * while hiding the unique_ptr. This is the non-const version.
		 */
		template<class VECTORTYPE, class ELEMENTTYPE>
		class UniquePtrVectorWrapper
		{
		public:
			UniquePtrVectorWrapper(VECTORTYPE& inVector) :
				mVector(&inVector)
			{ }

			using Iterator = UniquePtrVectorIterator<typename VECTORTYPE::iterator, ELEMENTTYPE>;

			Iterator begin() { return Iterator(mVector->begin()); }
			Iterator end() { return Iterator(mVector->end()); }

		private:
			VECTORTYPE* mVector;
		};


		/**
		 * Helper class to wrap a vector of unique_ptrs, allowing you to expose the vector to clients, while hiding the unique_ptr.
		 * This is the const version.
		 */
		template<class VECTORTYPE, class ELEMENTTYPE>
		class UniquePtrConstVectorWrapper
		{
		public:
			UniquePtrConstVectorWrapper(const VECTORTYPE& inVector) :
				mVector(&inVector)
			{ }

			using ConstIterator = UniquePtrVectorIterator<typename VECTORTYPE::const_iterator, const ELEMENTTYPE>;

			ConstIterator begin() const { return ConstIterator(mVector->begin()); }
			ConstIterator end() const { return ConstIterator(mVector->end()); }

		private:
			const VECTORTYPE* mVector;
		};
	}
}