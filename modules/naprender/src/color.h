#pragma once

#include <rtti/rtti.h>
#include <nap/configure.h>
#include <utility/dllexport.h>

namespace nap
{
	enum class NAPAPI EColorChannel : int
	{
		Red		= 0,
		Green	= 1,
		Blue	= 2,
		Alpha	= 3
	};


	/**
	 * Base class for all types of color
	 */
	class NAPAPI BaseColor
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
		int getNumberOfChannels() const											{ return mChannels; }

		/**
		 *	@return the size of a single channel in bytes
		 */
		int valueSize() const													{ return mValueSize; }

		/**
		 *	@return the color value data type
		 */
		virtual rtti::TypeInfo getValueType() const = 0;

		/**
		 * Converts the values associated with this color
		 * in to the compatible @color values.
		 */
		void convert(BaseColor& color) const;

		/**
		 * @return the data associated with the channel @index
		 */
		virtual const void* getData(int channel) const = 0;

		/**
		 *	@return the data associated with the channel @index
		 */
		virtual void* getData(int channel) = 0;

		/**
		 * @return the total size in bytes of the color
		 */
		int size() const														{ return mChannels * mValueSize; }

		/**
		 * Converts the color values in @from to @to
		 */
		static void convertColor(const BaseColor& from, BaseColor& to);

	private:
		int mChannels = 0;
		int mValueSize = 0;
	};

	/**
	 * Specific type of color where T defines the value type of the color
	 * and CHANNELS the number of channels associated with a color
	 * Colors are always packed in the following order RGBA. This class
	 * can also be used to store pointers to colors and can therefore act
	 * as a convenient wrapper around bitmap color values. All color types
	 * can be hashed for convenience. This makes most sense for 8 and 16 bit
	 * color types. For float colors a simple x/or is used
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
		rtti::TypeInfo getValueType() const override						{ return RTTI_OF(T).get_raw_type(); }

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
		 *	@return the data associated with the channel @index
		 */
		const void* getData(int channel) const override;

		/**
		*	@return the data associated with the channel @index
		*/
		void* getData(int channel) override;

		/**
		 *	@return if two color values are not similar
		 */
		bool operator== (const Color<T, CHANNELS>& rhs) const;

		/**
		 * @return if two color values are not similar
		 */
		bool operator!=(const Color<T, CHANNELS>& rhs) const				{ !(rhs == mValues); }
		
		/**
		*	Color values associated with this color
		*/
		std::array<T, CHANNELS> mValues;
	};

	/**
	 *	Utility class that provides a useful constructor for an RGB color
	 */
	template<typename T>
	class RGBColor : public Color<T, 3>
	{
		RTTI_ENABLE(Color)
	public:
		/**
		 *	Constructor that creates an RGB color based on the given values
		 */
		RGBColor(T red, T green, T blue) : Color<T,3>({red, green, blue})		{ }

		/**
		 *	Default constructor
		 */
		RGBColor() : Color<T,3>()												{ }
	};


	/**
	*	Utility class that provides a useful constructor for an RGBA color
	*/
	template<typename T>
	class RGBAColor : public Color<T, 4>
	{
		RTTI_ENABLE(Color)
	public:
		/**
		*	Constructor that creates an RGB color based on the given values
		*/
		RGBAColor(T red, T green, T blue, T alpha) : 
			Color<T, 4>({ red, green, blue, alpha })							{ }

		/**
		 *	Default constructor
		 */
		RGBAColor() : Color<T, 4>()												{ }
	};


	//////////////////////////////////////////////////////////////////////////
	// Type definitions for all supported color types
	// These color types can be used as a resource
	// The color values can be declared with the property: Values
	//////////////////////////////////////////////////////////////////////////

	using RGBColor8			= RGBColor<uint8>;
	using RGBColor16		= RGBColor<uint16>;
	using RGBColorFloat		= RGBColor<float>;
	using RGBAColor8		= RGBAColor<uint8>;
	using RGBAColor16		= RGBAColor<uint16>;
	using RGBAColorFloat	= RGBAColor<float>;


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
		assert(idx < this->getNumberOfChannels());
		return mValues[idx];
	}

	template<typename T, typename int CHANNELS>
	T nap::Color<T, CHANNELS>::getValue(EColorChannel channel) const
	{
		int idx = static_cast<int>(channel);
		assert(idx < this->getNumberOfChannels());
		return mValues[idx];
	}

	template<typename T, typename int CHANNELS>
	void nap::Color<T, CHANNELS>::setValue(EColorChannel channel, T value)
	{
		int idx = static_cast<int>(channel);
		assert(idx < this->getNumberOfChannels());
		mValues[idx] = value;
	}

	template<typename T, typename int CHANNELS>
	const void* nap::Color<T, CHANNELS>::getData(int channel) const
	{
		assert(channel < this->getNumberOfChannels());
		return &(mValues[channel]);
	}

	template<typename T, typename int CHANNELS>
	void* nap::Color<T, CHANNELS>::getData(int channel)
	{
		assert(channel < this->getNumberOfChannels());
		return &(mValues[channel]);
	}
}


//////////////////////////////////////////////////////////////////////////
// Hash functions for 8 and 16 bit integer colors
// No has support for floating point colors
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

	template <>
	struct hash<nap::Color<nap::uint16, 3>>
	{
		size_t operator()(const nap::Color<nap::uint16, 3>& v) const
		{
			return nap::uint64((
				(nap::uint64)v.getValue(nap::EColorChannel::Red) << 32		|
				(nap::uint64)v.getValue(nap::EColorChannel::Green) << 16	|
				(nap::uint64)v.getValue(nap::EColorChannel::Blue)));
		}
	};

	template <>
	struct hash<nap::Color<nap::uint16, 4>>
	{
		size_t operator()(const nap::Color<nap::uint16, 4>& v) const
		{
			return nap::uint64((
				(nap::uint64)v.getValue(nap::EColorChannel::Red) << 48		|
				(nap::uint64)v.getValue(nap::EColorChannel::Green) << 32	|
				(nap::uint64)v.getValue(nap::EColorChannel::Blue) << 16		|
				(nap::uint64)v.getValue(nap::EColorChannel::Alpha)));
		}
	};

	template <>
	struct hash<nap::Color<float, 3>>
	{
		size_t operator()(const nap::Color<float, 3>& v) const
		{
			std::size_t value1 = std::hash<float>{}(v.getValue(nap::EColorChannel::Red));
			std::size_t value2 = std::hash<float>{}(v.getValue(nap::EColorChannel::Green));
			std::size_t value3 = std::hash<float>{}(v.getValue(nap::EColorChannel::Blue));
			return value1 ^ value2 ^ value3;
		}
	};

	template <>
	struct hash<nap::Color<float, 4>>
	{
		size_t operator()(const nap::Color<float, 4>& v) const
		{
			std::size_t value1 = std::hash<float>{}(v.getValue(nap::EColorChannel::Red));
			std::size_t value2 = std::hash<float>{}(v.getValue(nap::EColorChannel::Green));
			std::size_t value3 = std::hash<float>{}(v.getValue(nap::EColorChannel::Blue));
			std::size_t value4 = std::hash<float>{}(v.getValue(nap::EColorChannel::Alpha));
			return value1 ^ value2 ^ value3 ^ value4;
		}
	};

	template <>
	struct hash<nap::RGBColor<nap::uint8>>
	{
		size_t operator()(const nap::RGBColor<nap::uint8>& v) const
		{
			return hash<nap::Color<nap::uint8, 3>>()(static_cast<nap::Color<nap::uint8,3>>(v));
		}
		
	};

	template <>
	struct hash<nap::RGBAColor<nap::uint8>>
	{
		size_t operator()(const nap::RGBAColor<nap::uint8>& v) const
		{
			return hash<nap::Color<nap::uint8, 4>>()(static_cast<nap::Color<nap::uint8, 4>>(v));
		}
	};

	template <>
	struct hash<nap::RGBColor<nap::uint16>>
	{
		size_t operator()(const nap::RGBColor<nap::uint16>& v) const
		{
			return hash<nap::Color<nap::uint16, 3>>()(static_cast<nap::Color<nap::uint16, 3>>(v));
		}

	};

	template <>
	struct hash<nap::RGBAColor<nap::uint16>>
	{
		size_t operator()(const nap::RGBAColor<nap::uint16>& v) const
		{
			return hash<nap::Color<nap::uint16, 4>>()(static_cast<nap::Color<nap::uint16, 4>>(v));
		}
	};

	template <>
	struct hash<nap::RGBColor<float>>
	{
		size_t operator()(const nap::RGBColor<float>& v) const
		{
			return hash<nap::Color<float, 3>>()(static_cast<nap::Color<float, 3>>(v));
		}

	};

	template <>
	struct hash<nap::RGBAColor<float>>
	{
		size_t operator()(const nap::RGBAColor<float>& v) const
		{
			return hash<nap::Color<float, 4>>()(static_cast<nap::Color<float, 4>>(v));
		}
	};
}
