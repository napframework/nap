#pragma once

#include <rtti/rtti.h>
#include <nap/configure.h>

namespace nap
{
	class Color
	{
		RTTI_ENABLE()
	public:
		/**
		* Base constructor associated with a color
		* @param channels the number of channels of the color
		* @param size the size in bytes of a single color channel
		*/
		Color(int channels, int size) : mChannels(channels), mValueSize(size) { }

		bool operator==(const Color& rhs) = delete;
		bool operator!=(const Color& rhs) = delete;

		/**
		 *	@return the number of channels associated with this color
		 */
		int numberOfChannels() const											{ return mChannels; }

		/**
		 *	@return the size of a single channel in bytes
		 */
		int valueSize() const													{ return mValueSize; }

		/**
		 *	@return the type of the color value
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
	 *	Color made up out of a Red, Green and Blue component
	 */
	template<typename T>
	class RGBColor : public Color
	{
		RTTI_ENABLE(Color)
	public:
		/**
		*	Constructor that creates a color based on red, green and blue
		*/
		RGBColor(T red, T green, T blue) : Color(3, sizeof(T))					{ mValues = { red, green, blue }; }

		/**
		*	Constructor that simply creates a 0 initialized color
		*/
		RGBColor() : Color(3, sizeof(T)) { }

		/**
		 *	@return the red color
		 */
		T getRed() const														{ return mValues[0]; }
		
		/**
		 *	@return the green color
		 */
		T getGreen() const														{ return mValues[1]; }
		
		/**
		 *	@return the blue color
		 */
		T getBlue() const														{ return mValues[2]; }

		/**
		 *	@return the type of the value
		 */
		rtti::TypeInfo getValueType() const override							{ return RTTI_OF(T); }

		/**
		 *	@return if two color values are not similar
		 */
		bool operator== (const RGBColor<T>& rhs) const;

		/**
		 * @return if two color values are not similar
		 */
		bool operator!=(const RGBColor<T>& rhs) const							{ !(rhs == mValues); }

		/**
		*	Color values associated with this color
		*/
		std::array<T, 3> mValues = { 0,0,0 };
	};


	/**
	* Color made up out of a Red, Green, Blue and Alpha component
	*/
	template<typename T>
	class RGBAColor : public Color
	{
		RTTI_ENABLE(Color)
	public:
		/**
		*	Constructor that creates a color based on red, green and blue
		*/
		RGBAColor(T red, T green, T blue, T alpha) : Color(4, sizeof(T))		{ mValues = { red, green, blue, alpha }; }

		/**
		*	Constructor that simply creates a 0 initialized color
		*/
		RGBAColor() : Color(4, sizeof(T)) { }

		/**
		*	@return the red color
		*/
		T getRed() const														{ return mValues[0]; }

		/**
		*	@return the green color
		*/
		T getGreen() const														{ return mValues[1]; }

		/**
		*	@return the blue color
		*/
		T getBlue() const														{ return mValues[2]; }

		/**
		 *	@return the alpha value
		 */
		T getAlpha() const														{ return mValues[4]; }

		/**
		 *	@return the type of the value
		 */
		rtti::TypeInfo getValueType() const override							{ return RTTI_OF(T); }

		/**
		 * @return if two RGBA colors are similar
		 */
		bool operator== (const RGBAColor<T>& rhs) const;

		/**
		 *	@return if two color values are not similar
		 */
		bool operator!=(const RGBColor<T>& rhs) const							{ !(rhs == mValues); }

		/**
		 *	Color values associated with this color
		 */
		std::array<T, 4> mValues = { 0,0,0,0 };
	};


	//////////////////////////////////////////////////////////////////////////
	// Type definitions for all supported vertex attribute types
	//////////////////////////////////////////////////////////////////////////

	using RGBColor8  = RGBColor<uint8>;
	using RGBAColor8 = RGBAColor<uint8>;


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	bool nap::RGBColor<T>::operator==(const RGBColor<T>& rhs) const
	{
		return (
			mValues[0] == rhs.mValues[0] &&
			mValues[1] == rhs.mValues[1] &&
			mValues[2] == rhs.mValues[2]);
	}

	template<typename T>
	bool nap::RGBAColor<T>::operator==(const RGBAColor<T>& rhs) const
	{
		return (
			mValues[0] == rhs.mValues[0] &&
			mValues[1] == rhs.mValues[1] &&
			mValues[2] == rhs.mValues[2] &&
			mValues[3] == rhs.mValues[3]);
	}
}


//////////////////////////////////////////////////////////////////////////
// Hash functions
//////////////////////////////////////////////////////////////////////////

namespace std
{
	template <>
	struct hash<nap::RGBColor<nap::uint8>>
	{
		size_t operator()(const nap::RGBColor<nap::uint8>& v) const
		{
			return nap::uint32((v.getRed() << 16 | v.getGreen() << 8 | v.getBlue()));
		}
	};

	template <>
	struct hash<nap::RGBAColor<nap::uint8>>
	{
		size_t operator()(const nap::RGBAColor<nap::uint8>& v) const
		{
			return nap::uint32((v.getRed() << 24 | v.getGreen() << 16 | v.getBlue() << 8 | v.getAlpha()));
		}
	};
}
