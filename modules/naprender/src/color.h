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
	 * Base class for all types of color. There are two types of colors:
	 * Colors that manage their own values and colors that point to values in memory
	 * Colors that point to values in memory have the suffix Data, where colors
	 * that manage their own values don't: ie: RGBColor8 is a color that holds
	 * 3 RGB uint8 (Byte) color values. RGBColorData8 on the other hand holds
	 * 3 pointers to RGB uint8 (Byte) values and does not own the data it points to.
	 * Colors that point to values in memory are useful when working with bitmaps and
	 * a copy is often unnecessary. They're also very useful to change the color value
	 * of a pixel in a bitmap directly, look at conversion for more info.
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
		 * @return if the data contained by this color points to values in memory
		 */
		virtual bool isPointer() const = 0;

		/**
		 * Converts and copies the values associated with this color in to @color.
		 * It's required that this color has an equal or higher amount of color channels
		 * Therefore this conversion is valid: RGBA8 to RGBFloat, but not: RGB8 to RGBAFloat
		 * This call asserts if the conversion can't be performed.
		 * When converting to and from float colors, normalized color values are used.
		 * Float values that do not fall within the 0-1 range are clamped
		 * When the target does not manage it's own color values, ie:
		 * holds pointers to color values in memory, the values that are pointed to are overridden.
		 * This makes it possible (for example) to write colors directly in to a bitmap without having
		 * to copy them over. 
		 * @param target the converted color value
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
		 * Converts the color values in @source Color to @target Color
		 * It's required that the from color has an equal or higher amount of color channels
		 * Therefore this conversion is valid: RGBA8 to RGBFloat, but not: RGB8 to RGBAFloat
		 * The following is also valid: RGBColorData16 to RGBColorFloat or, RGBAColorData8 to RGBColorData16
		 * This call asserts if the conversion can't be performed.
		 * When converting to and from float colors, normalized color values are used.
		 * Float values that do not fall within the 0-1 range are clamped
		 * When the target does not manage it's own color values, ie: 
		 * holds pointers to color values in memory, the values that are pointed to are overridden by the result of the conversion
		 * This makes it possible (for example) to write colors directly in to a bitmap without having
		 * to copy them over.
		 * @param source the color to convert
		 * @param target holds the converted color values
		 */
		static void convertColor(const BaseColor& source, BaseColor& target);

	private:
		int mChannels = 0;
		int mValueSize = 0;
	};

	/**
	 * Specific type of color where T defines the value type of the color.
	 * Every color has at least 1 or more channels.
	 * Colors are always packed in the following order RGBA. This class
	 * can also be used to store pointers to colors and can therefore act
	 * as a convenient wrapper around bitmap color values. All color types
	 * can be hashed for convenience. This makes most sense for 8 and 16 bit
	 * color types. For float colors a simple x/or is used. In order to hash
	 * colors you should use one of the explicitly defined successive color types, ie:
	 * RGBA8, R8, RGBFloat etc.
	 */
	template<typename T>
	class Color : public BaseColor
	{
		RTTI_ENABLE(BaseColor)
	public:
		/**
		* Constructor that simply creates a 0 initialized color
		*/
		Color(int channels) : BaseColor(channels, sizeof(T))							{ mValues.resize(channels, 0); }

		/**
		 * Constructor that creates a color based on a set number of values
		 * Note that the number of values needs to match the number of channels
		 * The order is important: RGBA
		 */
		Color(const std::vector<T>& colors) : 
			BaseColor(colors.size(), sizeof(T))											{ mValues = colors; }

		/**
		 *	@return the type of the value
		 */
		rtti::TypeInfo getValueType() const override									{ return RTTI_OF(T).get_raw_type(); }

		/**
		 *	@return if this color points to values in memory
		 */
		bool isPointer() const override;

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
		 *	@return all the values associated with this color
		 */
		const std::vector<T>& getValues() const											{ return mValues; }

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
		bool operator== (const Color<T>& rhs) const;

		/**
		 * @return if two color values are not similar
		 */
		bool operator!=(const Color<T>& rhs) const										{ !(rhs == mValues); }
		
		/**
		*	Color values associated with this color
		*/
		std::vector<T> mValues;
	};


	/**
	 *	Utility class that provides a useful constructor and accessors
	 */
	template<typename T>
	class RGBColor : public Color<T>
	{
		RTTI_ENABLE(Color)
	public:
		/**
		 *	Constructor that creates an RGB color based on the given values
		 */
		RGBColor(T red, T green, T blue) : Color<T>({red, green, blue})			{ }

		/**
		 *	Default constructor
		 */
		RGBColor() : Color<T>(3)												{ }

		/**
		* Sets the red channel to @value
		* @param value red color value
		*/
		void setRed(T value)													{ setValue(EColorChannel::Red, value); }

		/**
		* Sets the red channel to @value
		* @param value red color value
		*/
		T getRed() const														{ getValue(EColorChannel::Red); }

		/**
		* Sets the green channel to @value
		* @param value green color value
		*/
		void setGreen(T value)													{ setValue(EColorChannel::Green, value); }

		/**
		* Sets the green channel to @value
		* @param value green color value
		*/
		T getGreen() const														{ getValue(EColorChannel::Green); }

		/**
		* Sets the blue channel to @value
		* @param blue color value
		*/
		void setBlue(T value)													{ setValue(EColorChannel::Blue, value); }

		/**
		* Sets the blue channel to @value
		* @param blue color value
		*/
		T getBlue() const														{ getValue(EColorChannel::Blue); }
	};


	/**
	*	Utility class that provides a useful constructor and accessors
	*/
	template<typename T>
	class RGBAColor : public Color<T>
	{
		RTTI_ENABLE(Color)
	public:
		/**
		*	Constructor that creates an RGB color based on the given values
		*/
		RGBAColor(T red, T green, T blue, T alpha) : 
			Color<T>({ red, green, blue, alpha })								{ }

		/**
		 *	Default constructor
		 */
		RGBAColor() : Color<T>(4)												{ }

		/**
		 * Sets the red channel to @value
		 * @param value red color value
		 */
		void setRed(T value)													{ setValue(EColorChannel::Red, value); }

		/**
		* Sets the red channel to @value
		* @param value red color value
		*/
		T getRed() const														{ getValue(EColorChannel::Red); }

		/**
		* Sets the green channel to @value
		* @param value green color value
		*/
		void setGreen(T value)													{ setValue(EColorChannel::Green, value); }

		/**
		* Sets the green channel to @value
		* @param value green color value
		*/
		T getGreen() const														{ getValue(EColorChannel::Green); }

		/**
		* Sets the blue channel to @value
		* @param blue color value
		*/
		void setBlue(T value)													{ setValue(EColorChannel::Blue, value); }

		/**
		* Sets the blue channel to @value
		* @param blue color value
		*/
		T getBlue() const														{ getValue(EColorChannel::Blue); }

		/**
		* Sets the alpha channel to @value
		* @param value alpha color value
		*/
		void setAlpha(T value)													{ setValue(EColorChannel::Alpha, value); }

		/**
		* Sets the alpha channel to @value
		* @param value alpha color value
		*/
		T getAlpha() const														{ getValue(EColorChannel::Alpha); }
	};


	/**
	*	Utility class that provides a useful constructor for a single value color
	*/
	template<typename T>
	class RColor : public Color<T>
	{
		RTTI_ENABLE(Color)
	public:
		/**
		* Constructor that creates an R color based on the given value
		*/
		RColor(T value) : Color<T>(std::vector<T>(1, value))					{ }

		/**
		 *	Default constructor
		 */
		RColor() : Color<T>(1)													{ }

		/**
		* Sets the red channel to @value
		* @param value red color value
		*/
		void setRed(T value)													{ setValue(EColorChannel::Red, value); }

		/**
		* Sets the red channel to @value
		* @param value red color value
		*/
		T getRed() const														{ getValue(EColorChannel::Red); }
	};


	//////////////////////////////////////////////////////////////////////////
	// Type definitions for all supported hash-able color types
	// These color types can be used as a resource
	// The color values can be declared with the property: Values
	//////////////////////////////////////////////////////////////////////////

	using RGBColor8				= RGBColor<uint8>;
	using RGBColor16			= RGBColor<uint16>;
	using RGBColorFloat			= RGBColor<float>;
	using RGBAColor8			= RGBAColor<uint8>;
	using RGBAColor16			= RGBAColor<uint16>;
	using RGBAColorFloat		= RGBAColor<float>;
	using RColor8				= RColor<uint8>;
	using RColor16				= RColor<uint16>;
	using RColorFloat			= RColor<float>;


	//////////////////////////////////////////////////////////////////////////
	// Type definitions for all supported memory (data) color types
	// These colors are used to point to color values in memory 
	// These colors can't be serialized or used as a resource
	//////////////////////////////////////////////////////////////////////////

	using RGBColorData8			= RGBColor<uint8*>;
	using RGBColorData16		= RGBColor<uint16*>;
	using RGBColorDataFloat		= RGBColor<float*>;
	using RGBAColorData8		= RGBAColor<uint8*>;
	using RGBAColorData16		= RGBAColor<uint16*>;
	using RGBAColorDataFloat	= RGBAColor<float*>;
	using RColorData8			= RColor<uint8*>;
	using RColorData16			= RColor<uint16*>;
	using RColorDataFloat		= RColor<float*>;



	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	bool nap::Color<T>::operator==(const Color<T>& rhs) const
	{
		for (auto i = 0; i < mValues.size(); i++)
		{
			if (mValues[i] != rhs.mValues[i])
				return false;
		}
		return true;		
	}

	template<typename T>
	T& nap::Color<T>::getValue(EColorChannel channel)
	{
		int idx = static_cast<int>(channel);
		assert(idx < this->getNumberOfChannels());
		return mValues[idx];
	}

	template<typename T>
	T nap::Color<T>::getValue(EColorChannel channel) const
	{
		int idx = static_cast<int>(channel);
		assert(idx < this->getNumberOfChannels());
		return mValues[idx];
	}

	template<typename T>
	void nap::Color<T>::setValue(EColorChannel channel, T value)
	{
		int idx = static_cast<int>(channel);
		assert(idx < this->getNumberOfChannels());
		mValues[idx] = value;
	}

	template<typename T>
	const void* nap::Color<T>::getData(int channel) const
	{
		assert(channel < this->getNumberOfChannels());
		return &(mValues[channel]);
	}

	template<typename T>
	void* nap::Color<T>::getData(int channel)
	{
		assert(channel < this->getNumberOfChannels());
		return &(mValues[channel]);
	}

	template<typename T>
	bool nap::Color<T>::isPointer() const
	{
		return std::is_pointer<T>();
	}
}


//////////////////////////////////////////////////////////////////////////
// Hash functions for 8 and 16 bit integer colors
// Float colors are also hashable but something you probably should not do
//////////////////////////////////////////////////////////////////////////

namespace std
{
	template <>
	struct hash<nap::RColor<nap::uint8>>
	{
		size_t operator()(const nap::RColor<nap::uint8>& v) const
		{
			return hash<nap::uint8>()(v.getValue(nap::EColorChannel::Red));
		}
	};

	template <>
	struct hash<nap::RGBColor<nap::uint8>>
	{
		size_t operator()(const nap::RGBColor<nap::uint8>& v) const
		{
			return nap::uint32((
				v.getValue(nap::EColorChannel::Red) << 16  |
				v.getValue(nap::EColorChannel::Green) << 8 |
				v.getValue(nap::EColorChannel::Blue)));
		}
		
	};

	template <>
	struct hash<nap::RGBAColor<nap::uint8>>
	{
		size_t operator()(const nap::RGBAColor<nap::uint8>& v) const
		{
			return nap::uint32((
				v.getValue(nap::EColorChannel::Red) << 24	|
				v.getValue(nap::EColorChannel::Green) << 16 |
				v.getValue(nap::EColorChannel::Blue) << 8	|
				v.getValue(nap::EColorChannel::Alpha)));
		}
	};

	template <>
	struct hash<nap::RColor<nap::uint16>>
	{
		size_t operator()(const nap::RColor<nap::uint16>& v) const
		{
			return hash<nap::uint16>()(v.getValue(nap::EColorChannel::Red));
		}

	};

	template <>
	struct hash<nap::RGBColor<nap::uint16>>
	{
		size_t operator()(const nap::RGBColor<nap::uint16>& v) const
		{
			return nap::uint64((
				(nap::uint64)v.getValue(nap::EColorChannel::Red) << 32	 |
				(nap::uint64)v.getValue(nap::EColorChannel::Green) << 16 |
				(nap::uint64)v.getValue(nap::EColorChannel::Blue)));
		}

	};

	template <>
	struct hash<nap::RGBAColor<nap::uint16>>
	{
		size_t operator()(const nap::RGBAColor<nap::uint16>& v) const
		{
			return nap::uint64((
				(nap::uint64)v.getValue(nap::EColorChannel::Red) << 48		|
				(nap::uint64)v.getValue(nap::EColorChannel::Green) << 32	|
				(nap::uint64)v.getValue(nap::EColorChannel::Blue) << 16		|
				(nap::uint64)v.getValue(nap::EColorChannel::Alpha)));
		}
	};

	template <>
	struct hash<nap::RColor<float>>
	{
		size_t operator()(const nap::RColor<float>& v) const
		{
			return std::hash<float>{}(v.getValue(nap::EColorChannel::Red));
		}

	};

	template <>
	struct hash<nap::RGBColor<float>>
	{
		size_t operator()(const nap::RGBColor<float>& v) const
		{
			std::size_t value1 = std::hash<float>{}(v.getValue(nap::EColorChannel::Red));
			std::size_t value2 = std::hash<float>{}(v.getValue(nap::EColorChannel::Green));
			std::size_t value3 = std::hash<float>{}(v.getValue(nap::EColorChannel::Blue));
			return value1 ^ value2 ^ value3;
		}

	};

	template <>
	struct hash<nap::RGBAColor<float>>
	{
		size_t operator()(const nap::RGBAColor<float>& v) const
		{
			std::size_t value1 = std::hash<float>{}(v.getValue(nap::EColorChannel::Red));
			std::size_t value2 = std::hash<float>{}(v.getValue(nap::EColorChannel::Green));
			std::size_t value3 = std::hash<float>{}(v.getValue(nap::EColorChannel::Blue));
			std::size_t value4 = std::hash<float>{}(v.getValue(nap::EColorChannel::Alpha));
			return value1 ^ value2 ^ value3 ^ value4;
		}
	};
}
