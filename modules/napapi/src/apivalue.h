#pragma once

// External Includes
#include <rtti/rtti.h>
#include <nap/resource.h>
#include <iostream>

namespace nap
{
	// Forward Declares
	class APIArgument;

	/**
	* Base class of a value that can be given to or is constructed for an external environment.
	* This value can be copied, moved and serialized from and to JSON.
	*/
	class NAPAPI APIBaseValue : public Resource
	{
		friend class APIArgument;
		RTTI_ENABLE(Resource)
	public:
		APIBaseValue() = default;

		// Default constructor
		APIBaseValue(const rtti::TypeInfo& type, const std::string& name) : 
			mRepresentedType(type),
			mName(name)	{ }

		// Default destructor
		virtual ~APIBaseValue() = default;

		/**
		 * @return the actual type represented by this API value, ie: RTTI_OF(float) etc.	
		 */
		const rtti::TypeInfo& getRepresentedType() const	{ return mRepresentedType; }

		std::string mName;									///< Property: 'Name' name associated with the value

	protected:
		APIBaseValue(const rtti::TypeInfo& type) :
			mRepresentedType(type)							 { }

	private:
		rtti::TypeInfo mRepresentedType;					///< type of the embedded value, ie: RTTI_OF(float) etc.
	};


	/**
	* Represents an actual value that can be given to or constructed for an external environment.
	* This object owns T and can be moved, copied and serialized from and to JSON.
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
		APIValue(const std::string& name, T&& value);

		/**
		* Constructs a value of type T, a copy is made
		* @param value the value this object is constructed with
		*/
		APIValue(const std::string& name, const T& value);

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
	using APILong = APIValue<int64_t>;

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
	nap::APIValue<T>::APIValue(const std::string& name, T&& value) :
		APIBaseValue(RTTI_OF(T), name),
		mValue(std::move(value)) { }


	template<typename T>
	nap::APIValue<T>::APIValue(const std::string& name, const T& value) :
		APIBaseValue(RTTI_OF(T), name),
		mValue(value) { }


	template<typename T>
	nap::APIValue<T>::APIValue(const APIValue& other) :
		APIBaseValue(RTTI_OF(T), other.mName),
		mValue(other.mValue)
	{
		this->mID = other.mID;
	}


	template<typename T>
	nap::APIValue<T>::APIValue(APIValue&& other) :
		APIBaseValue(RTTI_OF(T)),
		mValue(std::move(other.mValue))
	{
		this->mName = std::move(other.mName);
		this->mID = std::move(other.mID);
	}


	template<typename T>
	nap::APIValue<T>& nap::APIValue<T>::operator=(APIValue<T>&& other)
	{
		mValue = std::move(other.mValue);
		mID = std::move(other.mID);
		mName = std::move(other.mName);
	}


	template<typename T>
	nap::APIValue<T>& nap::APIValue<T>::operator=(APIValue<T>& other)
	{
		mValue = other.mValue;
		mID = other.mID;
		mName = other.mName;
	}
}
