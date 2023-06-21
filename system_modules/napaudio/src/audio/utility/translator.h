/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <vector>
#include <functional>
#include <mathutils.h>

// Audio includes
#include <audio/utility/audiofunctions.h>


namespace nap
{
	namespace audio
	{
		
		/**
		 * An interface for a class that translates a value of a certain type into another value of the same type
		 */
		template<typename T>
		class Translator
		{
		public:
			virtual ~Translator() { }
			
			virtual T translate(const T& inputValue) = 0;
		
		private:
		};
		
		
		/**
		 * Translates one value to another using a vector of values as a lookup table, the input has to be between 0 and 1.
		 */
		template<typename T>
		class TableTranslator : public Translator<T>
		{
		public:
			/**
			 * a FillFunction takes a fractional index in range 0..1 as argument and returns the value of the curve at this index
			 */
			using FillFunction = std::function<T(T)>;
			
			/**
			 * default constructor
			 */
			TableTranslator(unsigned int size) { mTable.resize(size); }
			
			/**
			 * perform the translation, input is clipped between 0 and 1. and spread across the size of the table, interpolating between the values
			 */
			T translate(const T& inputValue) override final
			{
				return translate(inputValue, mTable);
			}
			
			/**
			 * fills the table with the result of a fill function with input values spaced between 0 and 1.
			 */
			void fill(FillFunction fillFunction)
			{
				for (int i = 0; i < mTable.size(); i++)
				{
					mTable[i] = fillFunction(i / T(mTable.size() - 1));
				}
			}
			
			/**
			 * perform the translation static, input is clipped between 0 and 1. and spread across the size of the table, interpolatinf between the values
			 */
			static inline T translate(const T& inputValue, const std::vector<T>& inTable)
			{
				// scale input accross table size
				T index = math::clamp(inputValue, 0.f, 1.f) * (inTable.size() - 1);
				
				// convert to integer
				int floor = index;
				
				// interpolate
				T frac = index - floor;
				if (frac > 0)
					return lerp(inTable[floor], inTable[floor + 1], frac);
				else
					return inTable[floor];
			}
			
		private:
			std::vector<T> mTable;
		};
		
		
		/**
		 * A convenience translator for equal power lookups, owns a table so it does not need to be provided
		 */
		template<typename T>
		class EqualPowerTranslator : public TableTranslator<T>
		{
		public:
			/**
			 * constructor
			 */
			EqualPowerTranslator(unsigned int size) : TableTranslator<T>(size)
			{
				// fills the table using equal power function
				TableTranslator<T>::fill([](T x) { return sin(x * math::PI_2); });
			}
		
		private:

		};
		
	}
}

