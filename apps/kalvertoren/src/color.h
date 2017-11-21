#pragma once

#include <rtti/rtti.h>
#include <nap/configure.h>

namespace nap
{
	enum class EColorChannel : int
	{
		Red		= 0,
		Green	= 1,
		Blue	= 2,
		Alpha	= 3
	};


	/**
	 * Base class for all types of color
	 */
	class BaseColor
	{
		RTTI_ENABLE()
	public:
		/**
		* Base constructor associated with a color
		* @param channels the number of channels of the color
		* @param size the size in bytes of a single color channel
		*/
		BaseColor(int channels, int size) : mChannels(channels), mValueSize(size) { }

		bool operator==(const BaseColor& rhs) = delete;
		bool operator!=(const BaseColor& rhs) = delete;

		/**
		 *	@return the number of channels associated with this color
		 */
		int numberOfChannels() const											{ return mChannels; }

		/**
		 *	@return the size of a single channel in bytes
		 */
		int valueSize() const													{ return mValueSize; }

		/**
		 *	@return the color value data type
		 */
		virtual rtti::TypeInfo getValueType() const = 0;

		/**
		 * @return the total size in bytes of the color
		 */
		int size() const														{ return mChannels * mValueSize; }

	private:
		int mChannels = 0;
		int mValueSize = 0;
	};


	/**
	 * Specific type of color where T defines the value type of the color
	 * and CHANNELS the number of channels associated with a color
	 * Colors are always packed in the following order RGBA. This class
	 * can also be used to store pointers to colors and can therefore act
	 * as a convenient wrapper around bitmap color values. Certain color types
	 * can also be used as a hash for a map
	 */
	template<typename T, typename int CHANNELS>
	class Color : public BaseColor
	{
		RTTI_ENABLE(BaseColor)
	public:
		/**
		*	Constructor that simply creates a 0 initialized color
		*/
		Color() : BaseColor(CHANNELS, sizeof(T))							{ mValues.fill(0); }

		/**
		 * Constructor that creates a color based on a set number of values
		 * Note that the number of values needs to match the number of channels
		 * The order is important: RGBA
		 */
		Color(const std::array<T, CHANNELS>& colors) : 
			BaseColor(CHANNELS, sizeof(T))									{ mValues = colors; }

		/**
		 *	@return the type of the value
		 */
		rtti::TypeInfo getValueType() const override						{ return RTTI_OF(T); }

		/**
		* @return the color value associated with @channel
		* This call asserts when the channel is not available
		*/
		T getValue(EColorChannel channel) const;

		/** 
		 * @return reference to the color value associated with @channel
		 * This call asserts when the channel is not available
		 */
		T& getValue(EColorChannel channel);

		/**
		 * Sets the color value for @channel
		 * This call asserts when the channel is not available
		 */
		void setValue(EColorChannel channel, T value);

		/**
		 *	@return if two color values are not similar
		 */
		bool operator== (const Color<T, CHANNELS>& rhs) const;

		/**
		 * @return if two color values are not similar
		 */
		bool operator!=(const Color<T, CHANNELS>& rhs) const				{ !(rhs == mValues); }
		
	protected:
		/**
		 *	Color values associated with this color
		 */
		std::array<T, CHANNELS> mValues;
	};


	//////////////////////////////////////////////////////////////////////////
	// Type definitions for all supported vertex attribute types
	//////////////////////////////////////////////////////////////////////////

	using RGBColor8			= Color<uint8,  3>;
	using RGBColor16		= Color<uint16, 3>;
	using RGBColorFloat		= Color<float,  3>;
	using RGBAColor8		= Color<uint8,  4>;
	using RGBAColor16		= Color<uint16, 4>;
	using RGBAColorFloat	= Color<float,  4>;


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T, typename int CHANNELS>
	bool nap::Color<T, CHANNELS>::operator==(const Color<T, CHANNELS>& rhs) const
	{
		for (auto i = 0; i < mValues.size(); i++)
		{
			if (mValues[i] != rhs.mValues[i])
				return false;
		}
		return true;		
	}

	template<typename T, typename int CHANNELS>
	T& nap::Color<T, CHANNELS>::getValue(EColorChannel channel)
	{
		int idx = static_cast<int>(channel);
		assert(idx < this->numberOfChannels());
		return mValues[idx];
	}

	template<typename T, typename int CHANNELS>
	T nap::Color<T, CHANNELS>::getValue(EColorChannel channel) const
	{
		int idx = static_cast<int>(channel);
		assert(idx < this->numberOfChannels());
		return mValues[idx];
	}

	template<typename T, typename int CHANNELS>
	void nap::Color<T, CHANNELS>::setValue(EColorChannel channel, T value)
	{
		int idx = static_cast<int>(channel);
		assert(idx < this->numberOfChannels());
		mValues[idx] = value;
	}
}


//////////////////////////////////////////////////////////////////////////
// Hash functions
//////////////////////////////////////////////////////////////////////////

namespace std
{
	template <>
	struct hash<nap::Color<nap::uint8, 3>>
	{
		size_t operator()(const nap::Color<nap::uint8, 3>& v) const
		{
			return nap::uint32((
				v.getValue(nap::EColorChannel::Red) << 16	| 
				v.getValue(nap::EColorChannel::Green) << 8  | 
				v.getValue(nap::EColorChannel::Blue)));
		}
	};

	template <>
	struct hash<nap::Color<nap::uint8, 4>>
	{
		size_t operator()(const nap::Color<nap::uint8, 4>& v) const
		{
			return nap::uint32((
				v.getValue(nap::EColorChannel::Red) << 24	| 
				v.getValue(nap::EColorChannel::Green) << 16 | 
				v.getValue(nap::EColorChannel::Blue) << 8	| 
				v.getValue(nap::EColorChannel::Alpha)));
		}
	};
}
