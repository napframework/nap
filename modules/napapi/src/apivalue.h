#pragma once

// External Includes
#include <rtti/rtti.h>
#include <nap/resource.h>

namespace nap
{
	// Forward Declares
	class APIArgument;

	/**
	* Base class of a value that can be given to or constructed for an external environment.
	* This value can be copied and move constructed.
	*/
	class NAPAPI APIBaseValue : public Resource
	{
		friend class APIArgument;
		RTTI_ENABLE(Resource)
	public:
		APIBaseValue() = default;

		// Default constructor
		APIBaseValue(const rtti::TypeInfo& type) : mRepresentedType(type) { }

		// Default destructor
		virtual ~APIBaseValue() = default;

	private:
		rtti::TypeInfo mRepresentedType;		///< type of the embedded value
	};


	/**
	* Represents an actual value that can be given to or constructed for an external environment.
	* This object owns T and can be moved and copied.
	*/
	template<typename T>
	class APIValue : public APIBaseValue
	{
		RTTI_ENABLE(APIBaseValue)
	public:
		APIValue();

		/**
		* Copy Constructor
		*/
		APIValue(APIValue&& other);

		/**
		* Move assignment constructor
		*/
		APIValue(const APIValue& other);

		/**
		* Move construct a value of type T
		* @param value the value given to this object
		*/
		APIValue(T&& value);

		/**
		* Constructs a value of type T, a copy is made
		* @param value the value this object is constructed with
		*/
		APIValue(const T& value);

		/**
		* Move assignment operator
		*/
		APIValue<T>& operator=(APIValue<T>&& other);

		/**
		* Copy assignment operator
		*/
		APIValue<T>& operator=(APIValue<T>& other);

		T mValue;				///< managed value
	};


	//////////////////////////////////////////////////////////////////////////
	// API Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using APIFloat = APIValue<float>;
	using APIBool = APIValue<bool>;
	using APIInt = APIValue<int>;
	using APIChar = APIValue<char>;
	using APIByte = APIValue<uint8_t>;
	using APIString = APIValue<std::string>;
	using APIDouble = APIValue<double>;
	using APILong = APIValue<long long>;

	using APIFloatArray = APIValue<std::vector<float>>;
	using APIIntArray = APIValue<std::vector<int>>;
	using APICharArray = APIValue<std::vector<char>>;
	using APIByteArray = APIValue<std::vector<uint8_t>>;
	using APIStringArray = APIValue<std::vector<std::string>>;
	using APIDoubleArray = APIValue<std::vector<double>>;


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	nap::APIValue<T>::APIValue(T&& value) :
		APIBaseValue(RTTI_OF(T)),
		mValue(std::move(value)) {  }


	template<typename T>
	nap::APIValue<T>::APIValue(const T& value) :
		APIBaseValue(RTTI_OF(T)),
		mValue(value) { }


	template<typename T>
	nap::APIValue<T>::APIValue(const APIValue& other) :
		APIBaseValue(RTTI_OF(T)),
		mValue(other.mValue)
	{
	}


	template<typename T>
	nap::APIValue<T>::APIValue(APIValue&& other) :
		APIBaseValue(RTTI_OF(T)),
		mValue(std::move(other.mValue))
	{
	}


	template<typename T>
	nap::APIValue<T>& nap::APIValue<T>::operator=(APIValue<T>&& other)
	{
		mValue = std::move(other.mValue);
	}


	template<typename T>
	nap::APIValue<T>& nap::APIValue<T>::operator=(APIValue<T>& other)
	{
		mValue = other.mValue;
	}
}
