/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
		/**
		 * Required constructor
		 * @param type the type of the managed value, for example: RTTI_OF(float) etc.
		 * @param name the name of this api value
		 */
		APIBaseValue(const rtti::TypeInfo& type, const std::string& name) :
			mName(name),
            mRepresentedType(type) { }

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
		 * Move Constructor
		 * @param other value to move
		 */
		APIValue(APIValue&& other);

		/**
		 * Copy constructor
		 * @param other value to copy
		 */
		APIValue(const APIValue& other);

		/**
		 * Move construct a value of type T
		 * @param name the name of the value
		 * @param value the value given to this object
		 */
		APIValue(const std::string& name, T&& value);

		/**
		 * Constructs a value of type T, a copy is made
		 * @param name the name of the value
		 * @param value the value this object is constructed with
		 */
		APIValue(const std::string& name, const T& value);

		/**
		 * Move assignment operator
		 * @param other api value to move
		 */
		APIValue<T>& operator=(APIValue<T>&& other);

		/**
		 * Copy assignment operator
		 * @param other api value to copy
		 */
		APIValue<T>& operator=(const APIValue<T>& other);

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
	using APIBoolArray = APIValue<std::vector<bool>>;
	using APIIntArray = APIValue<std::vector<int>>;
	using APICharArray = APIValue<std::vector<char>>;
	using APIByteArray = APIValue<std::vector<uint8_t>>;
	using APIStringArray = APIValue<std::vector<std::string>>;
	using APIDoubleArray = APIValue<std::vector<double>>;
	using APILongArray = APIValue<std::vector<int64_t>>;


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
		if (this != &other)
		{
			mValue = std::move(other.mValue);
			mID = std::move(other.mID);
			mName = std::move(other.mName);
		}

		return *this;
	}


	template<typename T>
	nap::APIValue<T>& nap::APIValue<T>::operator=(const APIValue<T>& other)
	{
		if (this != &other)
		{
			mValue = other.mValue;
			mID = other.mID;
			mName = other.mName;
		}
		return *this;
	}
}
