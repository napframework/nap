/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <rtti/rtti.h>
#include <nap/numeric.h>
#include <utility/dllexport.h>
#include <glm/glm.hpp>

namespace nap
{
	/**
	 *	Used by the various color classes to map a channel to an index
	 */
	enum class EColorChannel : int
	{
		Red		= 0,				///< Red Color Channel: 0
		Green	= 1,				///< Green Color Channel: 1
		Blue	= 2,				///< Blue Color Channel: 2
		Alpha	= 3					///< Alpha Color Channel: 3
	};


	/**
	 * Base class for all types of color. There are two types of colors:
	 * Colors that manage their own values and colors that point to values in memory.
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

        virtual ~BaseColor() = default;
        
        
		bool operator==(const BaseColor& rhs) = delete;
		bool operator!=(const BaseColor& rhs) = delete;

		/**
		 * converts color type A in to type B
		 */
		using Converter = std::function<void(const BaseColor&, BaseColor&, int)>;

		/**
		 *	@return the number of channels associated with this color
		 */
		inline int getNumberOfChannels() const									{ return mChannels; }

		/**
		 *	@return the size of a single channel in bytes
		 */
		inline int valueSize() const											{ return mValueSize; }

		/**
		 *	@return the color value data type
		 */
		virtual rtti::TypeInfo getValueType() const = 0;

		/**
		 * @return if the data contained by this color points to values in memory
		 */
		virtual bool isPointer() const = 0;

		/**
		 * Converts and copies the values associated with this color in to the target color.
		 * It's required that the source has an equal or higher amount of color channels compared to target.
		 * Therefore this conversion is valid: RGBA8 to RGBFloat, but not: RGB8 to RGBAFloat.
		 * This call asserts if the conversion can't be performed..
		 * When converting to and from float colors, normalized color values are used.
		 * Float values that do not fall within the 0-1 range are clamped.
		 * When the target does not manage it's own color values, ie:
		 * holds pointers to color values in memory, the values that are pointed to in target are overridden.
		 * This makes it possible (for example) to write colors directly in to a bitmap without having
		 * to copy them over. 
		 *
		 * This is therefore valid (source)RGBAColorData8 -> (target)RGBColor8, the target will have a copy of the values pointed to by it's source
		 * The following is also valid: (source)RGBColor8 -> (target)RGBColorData8, the target will have the values it points to replaced by the converted color value
		 *
		 * @param target the converted color value
		 */
		void convert(BaseColor& target) const;

		/**
		* @return a color converter to convert source color in to target color, nullptr if no such converter exists
		* Use this call when dealing with the same color conversion multiple times in, for example, a loop.
		*/
		Converter getConverter(const BaseColor& target) const;

		/**
		 * returns a (converted) color of type T.
		 * It's required that the color that is returned has a lower amount of color channels.
		 * Therefore this conversion is valid: RGBA8 to RGBFloat, but not: RGB8 to RGBAFloat.
		 * This call asserts if the conversion can't be performed.
		 * When converting to and from float colors, normalized color values are used.
		 * Float values that do not fall within the 0-1 range are clamped
		 * This call won't work with colors that point to values in memory! Valid options for T are: RGBColor8, RColor8, RGBColorFloat etc.
		 * 
		 * Example:
		 *
		 *~~~~~{.cpp}
		 * // Create RGBA 8 bit color
		 * RGBAColor8 eight_bit_color = { 0xC8, 0x69, 0x69, 0xFF };
		 *
		 * // Convert to RGBA float color
		 * RGBAColorFloat as_float_color = eight_bit_color.convert<RGBAColorFloat>();
		 *~~~~~
		 *
		 * @return the converted color
		 */
		template<typename T>
		T convert() const;

		/**
		 * @param channel number of the channel to get the data for
		 * @return the data associated with the channel at the index
		 */
		virtual const void* getData(int channel) const = 0;

		/**
		 * @param channel number of the channel to get the data for
		 * @return the data associated with the channel at the index
		 */
		virtual void* getData(int channel) = 0;

		/**
		 * @return the total size in bytes of the color
		 */
		int size() const														{ return mChannels * mValueSize; }

		/**
		 * Converts the color values in source Color to target Color.
		 * It's required that the from color has an equal or higher amount of color channels.
		 * Therefore this conversion is valid: RGBA8 to RGBFloat, but not: RGB8 to RGBAFloat.
		 * The following is also valid: RGBColorData16 to RGBColorFloat or, RGBAColorData8 to RGBColorData16.
		 * This call asserts if the conversion can't be performed.
		 * When converting to and from float colors, normalized color values are used.
		 * Float values that do not fall within the 0-1 range are clamped
		 * When the target does not manage it's own color values, ie: 
		 * holds pointers to color values in memory, the values that are pointed to are overridden by the result of the conversion.
		 * This makes it possible (for example) to write colors directly in to a bitmap without having
		 * to copy them over.
		 * @param source the color to convert
		 * @param target holds the converted color values
		 */
		static void convertColor(const BaseColor& source, BaseColor& target);

		/**
		* Converts the color values in source Color to target Color using the given conversion method
		* It's required that the from color has an equal or higher amount of color channels
		* Therefore this conversion is valid: RGBA8 to RGBFloat, but not: RGB8 to RGBAFloat
		* The following is also valid: RGBColorData16 to RGBColorFloat or, RGBAColorData8 to RGBColorData16
		* When converting to and from float colors, normalized color values are used.
		* Float values that do not fall within the 0-1 range are clamped
		* When the target does not manage it's own color values, ie:
		* holds pointers to color values in memory, the values that are pointed to are overridden by the result of the conversion
		* This makes it possible (for example) to write colors directly in to a bitmap without having
		* to copy them over.
		*
		* This call is faster than the default convertColor method and is recommended to be used in loops!
		* Make sure to call getConverter() before staring the loop to acquire the right conversion method
		* 
		* @param source the color to convert
		* @param target holds the converted color values
		* @param converter the method used to convert source color into target color
		*/
		static void convertColor(const BaseColor& source, BaseColor& target, const Converter& converter);

		/**
		 * @return a color converter to convert source color in to target color, nullptr if no such converter exists
		 * Use this call when dealing with the same color conversion multiple times in, for example, a loop
		 */
		static Converter getConverter(const BaseColor& source, const BaseColor& target);

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
	template<typename T, int CHANNELS>
	class Color : public BaseColor
	{
	public:
		/**
		* Constructor that simply creates a 0 initialized color
		*/
		Color() : BaseColor(CHANNELS, sizeof(T))										{ mValues.fill(0); }

		/**
		 * Constructor that creates a color based on a set number of values.
		 * Note that the number of values needs to match the number of channels.
		 * The order is important: RGBA
		 */
		Color(const std::array<T, CHANNELS>& colors) : 
			BaseColor(CHANNELS, sizeof(T)), mValues(colors)								{ }

		/**
		 *	@return the type of the value
		 */
		rtti::TypeInfo getValueType() const override									{ return RTTI_OF(T).get_raw_type(); }

		/**
		 *	@return if this color points to values in memory
		 */
		bool isPointer() const override;

		/**
		 * Returns the color value associated with a given channel.
		 * This call asserts when the channel is not available.
		 * @param channel the channel to get the value for
		 * @return the color value associated with the specified channel
		 */
		T getValue(EColorChannel channel) const;

		/** 
		 * @param channel the channel to get the value for
		 * @return reference to the color value associated with the specified channel
		 * This call asserts when the channel is not available
		 */
		T& getValue(EColorChannel channel);

		/**
		 * Sets the color value for the incoming channel.
		 * This call asserts when the channel is not available.
		 * @param channel the color channel to set
		 * @param value the new color value
		 */
		void setValue(EColorChannel channel, T value);

		/**
		 *	@return all the values associated with this color
		 */
		const std::array<T, CHANNELS>& getValues() const											{ return mValues; }

		/**
		 *	@return pointer to the beginning of the data
		 */
		T* getData()																	{ return mValues.data(); }

		/**
		 *	@return pointer to the begging of the data
		 */
		const T* getData() const														{ return mValues.data(); }

		/**
		 * Set the color data associated with this color.
		 * This call assumes the data is of the right size and length.
		 * When this color points to a location in memory, that memory location is copied.
		 * Otherwise the actual values are copied over.
		 * @param data the color data to copy, behind the scenes a memcopy is performed
		 */
		void setData(T* data);

		/**
		 * Computes the distance between this and another color in Euclidean space. The result is not squared
		 * @param other the color to compare against
		 * @return the non squared distance between this and another color
		 */
		float getDistance(const Color<T, CHANNELS>& other) const;

		/**
		 * @return if two color values are similar.
		 * Performs a value comparison when the color is not a pointer.
		 * Otherwise a pointer comparison.
		 */
		bool operator== (const Color<T, CHANNELS>& rhs) const;

		/**
		 * @return if two color values are not similar
		 */
		bool operator!=(const Color<T, CHANNELS>& rhs) const							{ return !(rhs == mValues); }
		
		/**
		 * @return if the color value is less than another color
		 * Won't work when the color is a pointer
		 */
		bool operator<(const Color<T, CHANNELS>& rhs) const;

		/**
		 * @return if the color value higher than another color
		 * Won't work when the color is a pointer
		 */
		bool operator>(const Color<T, CHANNELS>& rhs) const								{ return rhs < *this; }

		/**
		 * @return if the color value is less or equal to another color
		 * Won't work when the color is a pointer
		 */
		bool operator<=(const Color<T, CHANNELS>& rhs) const							{ return !(*this > rhs); }

		/**
		 * @return if the color value is higher or equal to another color
		 * Won't work when the color is a pointer
		 */
		bool operator>=(const Color<T, CHANNELS>& rhs) const							{ return !(*this < rhs); }

		/**
		 * Array subscript overload. Does not perform a bounds check!
		 * @return the color at index
		 */
		T& operator[](std::size_t index)												{ return mValues[index]; }

		/**
		 * Array subscript overload. Does not perform a bounds check!
		 * @return the color at index
		 */
		const T& operator[](std::size_t index) const									{ return mValues[index]; }

		/**
		 * Color values associated with this color
		 */
		std::array<T, CHANNELS> mValues;

	private:
		/**
		 * @return the data associated with the channel specified by index
		 */
		const void* getData(int channel) const override;

		/**
		 * @return the data associated with the channel specified by index
		 */
		void* getData(int channel) override;
	};

    template<typename T>
    using RGBColorBase  = Color<T,3>;
    
    template<typename T>
    using RGBAColorBase = Color<T,4>;
   
    template<typename T>
    using RColorBase    = Color<T,1>;
        
	/**
	 * Color that has a red, green and blue channel of type T
	 */
	template<typename T>
	class RGBColor : public Color<T, 3>
	{
		RTTI_ENABLE(BaseColor)
	public:
		/**
		 *	Constructor that creates an RGB color based on the given values
		 */
		RGBColor(T red, T green, T blue) : Color<T, 3>({red, green, blue})		{ }

		/**
		 *	Default constructor
		 */
		RGBColor() : Color<T, 3>()												{ }

		/**
		* Sets the red channel to the incoming value
		* @param value red color value
		*/
        void setRed(T value)													{ Color<T,3>::setValue(EColorChannel::Red, value); }

		/*
		 * @return the red color value
		 */
        T getRed() const														{ return Color<T,3>::getValue(EColorChannel::Red); }

		/**
		* Sets the green channel to the incoming value
		* @param value green color value
		*/
		void setGreen(T value)													{ Color<T,3>::setValue(EColorChannel::Green, value); }

		/**
		 * @return the green color value
		 */
		T getGreen() const														{ return Color<T,3>::getValue(EColorChannel::Green); }

		/**
		 * Sets the blue channel to the given color value
		 * @param value color value
		 */
		void setBlue(T value)													{ Color<T,3>::setValue(EColorChannel::Blue, value); }

		/**
		 * @return the blue color value
		 */
		T getBlue() const														{ return Color<T,3>::getValue(EColorChannel::Blue); }

		/**
		 * @return the color as a vec3 (float)
		 */
		glm::vec3 toVec3() const;
	};


	/**
	 * Color that has a red, green, blue and alpha channel of type T
	 */
	template<typename T>
	class RGBAColor : public Color<T,4>
	{
		RTTI_ENABLE(BaseColor)
	public:
		/**
		*	Constructor that creates an RGB color based on the given values
		*/
		RGBAColor(T red, T green, T blue, T alpha) :
			Color<T, 4>({ red, green, blue, alpha }) { }

		/**
		*	Default constructor
		*/
		RGBAColor() : Color<T, 4>()												{ }

		/**
		 * Sets the red channel to the incoming value
		 * @param value red color value
		 */
		void setRed(T value)													{ Color<T,4>::setValue(EColorChannel::Red, value); }

		/*
		 * @return the red color value
		 */
		T getRed() const														{ return Color<T,4>::getValue(EColorChannel::Red); }

		/**
		 * Sets the green channel to the given value
		 * @param value green color value
		 */
		void setGreen(T value)													{ Color<T,4>::setValue(EColorChannel::Green, value); }

		/*
		* @return the green color value
		*/
		T getGreen() const														{ return Color<T,4>::getValue(EColorChannel::Green); }

		/**
		 * Sets the blue channel to the given value.
		 * @param value new color value
		 */
		void setBlue(T value)													{ Color<T,4>::setValue(EColorChannel::Blue, value); }

		/*
		 * @return the blue color value
		 */
		T getBlue() const														{ return Color<T,4>::getValue(EColorChannel::Blue); }

		/**
		* Sets the alpha channel to the incoming value
		* @param value alpha color value
		*/
		void setAlpha(T value)													{ Color<T,4>::setValue(EColorChannel::Alpha, value); }

		/*
		 * @return the alpha color value
		 */
		T getAlpha() const														{ return Color<T,4>::getValue(EColorChannel::Alpha); }

		/**
		 *	@return the color as a vec4 (float)
		 */
		glm::vec4 toVec4() const;
	};

	/**
	* Color that has only one value associated with it.
	* The red color can be associated (for example) with height or a stencil value
	*/
	template<typename T>
	class RColor : public Color<T,1>
	{
		RTTI_ENABLE(BaseColor)
	public:
		/**
		* Constructor that creates an R color based on the given value
		*/
		RColor(T value) : Color<T, 1>({ value }) { }

		/**
		*	Default constructor
		*/
		RColor() : Color<T, 1>() { }

		/**
		* Sets the red channel to the incoming value
		* @param value red color value
		*/
		void setRed(T value)													{ Color<T,1>::setValue(EColorChannel::Red, value); }

		/*
		* @return the red color value
		*/
		T getRed() const														{ return Color<T,1>::getValue(EColorChannel::Red); }
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

	template<typename T, int CHANNELS>
	bool nap::Color<T, CHANNELS>::operator==(const Color<T, CHANNELS>& rhs) const
	{
		for (auto i = 0; i < CHANNELS; i++)
		{
			if (mValues[i] != rhs.mValues[i])
				return false;
		}
		return true;		
	}

	template<typename T, int CHANNELS>
	bool nap::Color<T, CHANNELS>::operator<(const Color<T, CHANNELS>& rhs) const
	{
		std::hash<Color<T, CHANNELS>> lhs_hash;
		std::hash<Color<T, CHANNELS>> rhs_hash;

		return lhs_hash(*this) < rhs_hash(rhs);
	}

	template<typename T, int CHANNELS>
	T& nap::Color<T, CHANNELS>::getValue(EColorChannel channel)
	{
		int idx = static_cast<int>(channel);
		assert(idx < CHANNELS);
		return mValues[idx];
	}

	template<typename T, int CHANNELS>
	T nap::Color<T, CHANNELS>::getValue(EColorChannel channel) const
	{
		int idx = static_cast<int>(channel);
		assert(idx < CHANNELS);
		return mValues[idx];
	}

	template<typename T, int CHANNELS>
	void nap::Color<T, CHANNELS>::setValue(EColorChannel channel, T value)
	{
		int idx = static_cast<int>(channel);
		assert(idx < CHANNELS);
		mValues[idx] = value;
	}

	template<typename T, int CHANNELS>
	const void* nap::Color<T, CHANNELS>::getData(int channel) const
	{
		assert(channel < CHANNELS);
		return &(mValues[channel]);
	}

	template<typename T, int CHANNELS>
	void* nap::Color<T, CHANNELS>::getData(int channel)
	{
		assert(channel < CHANNELS);
		return &(mValues[channel]);
	}

	template<typename T, int CHANNELS>
	bool nap::Color<T, CHANNELS>::isPointer() const
	{
		return std::is_pointer<T>();
	}

	template<typename T>
	T nap::BaseColor::convert() const
	{
		T color;
		assert(!(color.isPointer()));
		BaseColor::convertColor(*this, color);
		return color;
	}

	template<typename T, int CHANNELS>
	void nap::Color<T, CHANNELS>::setData(T* data)
	{
		memcpy(mValues.data(), data, sizeof(T) * CHANNELS);
	}

	template<typename T, int CHANNELS>
	float nap::Color<T, CHANNELS>::getDistance(const Color<T, CHANNELS>& other) const
	{
		float dist(0);
		for (int i = 0; i < CHANNELS; i++)
		{
			float diff = (float)(this->mValues[i]) - (float)(other.mValues[i]);
			dist += pow(diff,2);
		}
		return dist;
	}

	template<typename T>
	glm::vec3 nap::RGBColor<T>::toVec3() const
	{
		return 
		{ 
			(float)(RGBColor<T>::mValues[0]),
			(float)(RGBColor<T>::mValues[1]),
			(float)(RGBColor<T>::mValues[2])
		};
	};

	template<typename T>
	glm::vec4 nap::RGBAColor<T>::toVec4() const
	{
		return
		{
			(float)(RGBAColor<T>::mValues[0]),
			(float)(RGBAColor<T>::mValues[1]),
			(float)(RGBAColor<T>::mValues[2]),
			(float)(RGBAColor<T>::mValues[3])
		};
	}
}


//////////////////////////////////////////////////////////////////////////
// Hash functions for 8 and 16 bit integer colors
// Float colors are also hashable but something you probably should not do
//////////////////////////////////////////////////////////////////////////

namespace std
{
	template<>
	struct hash <nap::Color<nap::uint8, 1>>
	{
		size_t operator()(const nap::Color<nap::uint8, 1>& v) const
		{
			return hash<nap::uint8>()(v.getValue(nap::EColorChannel::Red));
		}
	};


	template <>
	struct hash<nap::RColor<nap::uint8>>
	{
		size_t operator()(const nap::RColor<nap::uint8>& v) const
		{
			return hash<nap::Color<nap::uint8, 1>>()(static_cast<nap::Color<nap::uint8, 1>>(v));
		}
	};


	template <>
	struct hash<nap::Color<nap::uint8, 3>>
	{
		size_t operator()(const nap::Color<nap::uint8, 3>& v) const
		{
			return nap::uint32((
				v.getValue(nap::EColorChannel::Red) << 16 |
				v.getValue(nap::EColorChannel::Green) << 8 |
				v.getValue(nap::EColorChannel::Blue)));
		}

	};


	template <>
	struct hash<nap::RGBColor<nap::uint8>>
	{
		size_t operator()(const nap::RGBColor<nap::uint8>& v) const
		{
			return hash<nap::Color<nap::uint8, 3>>()(static_cast<nap::Color<nap::uint8, 3>>(v));
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
	struct hash<nap::RGBAColor<nap::uint8>>
	{
		size_t operator()(const nap::RGBAColor<nap::uint8>& v) const
		{
			return hash<nap::Color<nap::uint8, 4>>()(static_cast<nap::Color<nap::uint8, 4>>(v));
		}
	};

	template <>
	struct hash<nap::Color<nap::uint16, 1>>
	{
		size_t operator()(const nap::Color<nap::uint16, 1>& v) const
		{
			return hash<nap::uint16>()(v.getValue(nap::EColorChannel::Red));
		}

	};

	template <>
	struct hash<nap::RColor<nap::uint16>>
	{
		size_t operator()(const nap::RColor<nap::uint16>& v) const
		{
			return hash<nap::Color<nap::uint16, 1>>()(static_cast<nap::Color<nap::uint16, 1>>(v));
		}

	};

	template <>
	struct hash<nap::Color<nap::uint16, 3>>
	{
		size_t operator()(const nap::Color<nap::uint16, 3>& v) const
		{
			return nap::uint64((
				(nap::uint64)v.getValue(nap::EColorChannel::Red) << 32 |
				(nap::uint64)v.getValue(nap::EColorChannel::Green) << 16 |
				(nap::uint64)v.getValue(nap::EColorChannel::Blue)));
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
	struct hash<nap::RGBAColor<nap::uint16>>
	{
		size_t operator()(const nap::RGBAColor<nap::uint16>& v) const
		{
			return hash<nap::Color<nap::uint16, 4>>()(static_cast<nap::Color<nap::uint16, 4>>(v));
		}
	};

	template <>
	struct hash<nap::Color<float, 1>>
	{
		size_t operator()(const nap::Color<float, 1>& v) const
		{
			return std::hash<float>{}(v.getValue(nap::EColorChannel::Red));
		}

	};

	template <>
	struct hash<nap::RColor<float>>
	{
		size_t operator()(const nap::RColor<float>& v) const
		{
			return hash<nap::Color<float, 1>>()(static_cast<nap::Color<float, 1>>(v));
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
	struct hash<nap::RGBColor<float>>
	{
		size_t operator()(const nap::RGBColor<float>& v) const
		{
			return hash<nap::Color<float, 3>>()(static_cast<nap::Color<float, 3>>(v));
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
	struct hash<nap::RGBAColor<float>>
	{
		size_t operator()(const nap::RGBAColor<float>& v) const
		{
			return hash<nap::Color<float, 4>>()(static_cast<nap::Color<float, 4>>(v));
		}
	};

	template <>
	struct hash<nap::EColorChannel>
	{
		size_t operator()(const nap::EColorChannel& v) const
		{
			return hash<int>()(static_cast<int>(v));
		}
	};
}
