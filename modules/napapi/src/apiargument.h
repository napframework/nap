#pragma once

#include <rtti/rtti.h>

namespace nap
{
	// Forward Declares
	class BaseAPIValue;

	/**
	 * Wrapper around an APIValue that can be given to or is received from an external environment.
	 * This object can not be copied or moved but the APIValue can be moved and copied.
	 * To access the managed APIValue use the get() or getCopy() methods.
	 * Use the other methods to translate this argument directly into more common types such as int, string, vector<T> etc.
	 */
	class NAPAPI APIArgument final
	{
	public:
		/**
		 * Arguments can only be constructed using a value
		 * @param value the value associated with this argument, owned by this object.
		 */
		APIArgument(std::unique_ptr<BaseAPIValue> value) : mAPIValue(std::move(value)) { }

		// Default destructor
		~APIArgument() = default;

		/**
		 * Copy is not allowed
		 */
		APIArgument(APIArgument&) = delete;
		APIArgument& operator=(const APIArgument&) = delete;

		/**
		 * Returns the actual type of the value managed by this argument: RTTI_OF(float), RTTI_OF(int) etc.
		 * Note that when this argument represents an array the return value is the value type contained by the array.
		 * For example, the return type of an array, say std::vector<float> is RTTI_OF(float).
		 * @return the value type represented by this argument.
		 */
		const rtti::TypeInfo getValueType() const;

		/**
		 * Returns the API value as a pointer of type T, where T is of type APIValue
		 * @return pointer to the API value, nullptr if the api value is not of type T.
		 */
		template<typename T>
		T* get();

		/**
		 * Returns the API value as a pointer of type T, where T is of type APIValue
		 * @return pointer to the API value, nullptr if the api value is not of type T.
		 */
		template<typename T>
		const T* get() const;
		
		/**
		 * @return a copy of the API value as T, where T is of type APIValue	
		 */
		template<typename T>
		T getCopy();

		/**
		 * @return if this argument holds an array
		 */
		bool isArray() const;

		/**
		 * Returns the API value as a vector of value type T, nullptr if the argument isn't an array or value types don't match.
		 */
		template<typename T>
		const std::vector<T>* asArray() const;

		/**
		 * Returns the API value as a vector of value type T, nullptr if the argument isn't an array or value types don't match.
		 */
		template<typename T>
		std::vector<T>* asArray();

		/**
		 * @return if argument's value is a string 	
		 */
		bool isString() const;

		/**
		 * @return this argument's value as a string	
		 */
		std::string asString() const;

		/**
		 * @return if this argument's value is a char
		 */
		bool isChar() const;

		/**
		 * @return this argument's value as a char
		 */
		char asChar() const;

		/**
		 *	@return if this argument's value is a float
		 */
		bool isFloat() const;

		/**
		 *	@return this argument's value as a float
		 */
		float asFloat() const;
		
		/**
		 *	@return if this argument's value is a float
		 */
		bool isDouble() const;

		/**
		 *	@return this argument's value as a float
		 */
		float asDouble() const;

		/**
		 * @return if this argument's value is an int
		 */
		bool isInt() const;

		/**
		 * @return this argument's value as an int
		 */
		int asInt() const;

		/**
		 *	@return if this argument's value is a bool
		 */
		bool isBool() const;

		/**
		 *	@return this argument's value as a bool
		 */
		bool asBool() const;

	private:
		std::unique_ptr<BaseAPIValue> mAPIValue = nullptr;	///< API Value represented by this argument
 	};


	//////////////////////////////////////////////////////////////////////////
	// API Values
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Base class of a value that can be given to or constructed for an external environment.
	 * This value can be copied and move constructed.
	 */
	class NAPAPI BaseAPIValue
	{
		friend class APIArgument;
		RTTI_ENABLE()
	public:
		// Default constructor
		BaseAPIValue(const rtti::TypeInfo& type) : mRepresentedType(type)	{ }

		// Default destructor
		virtual ~BaseAPIValue() = default;

	private:
		rtti::TypeInfo mRepresentedType;		///< type of the embedded value
	};


	/**
	 * Represents an actual value that can be given to or constructed for an external environment.
	 * This object owns T and can be moved and copied.
	 */
	template<typename T>
	class APIValue : public BaseAPIValue
	{
		RTTI_ENABLE(BaseAPIValue)
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

	using APIFloat			= APIValue<float>;
	using APIBool			= APIValue<bool>;
	using APIInt			= APIValue<int>;
	using APIChar			= APIValue<char>;
	using APIString			= APIValue<std::string>;
	using APIDouble			= APIValue<double>;
	using APILong			= APIValue<long long>;

	using APIFloatArray		= APIValue<std::vector<float>>;
	using APIIntArray		= APIValue<std::vector<int>>;
	using APICharArray		= APIValue<std::vector<char>>;
	using APIStringArray	= APIValue<std::vector<std::string>>;
	using APIDoubleArray	= APIValue<std::vector<double>>;


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	nap::APIValue<T>::APIValue(T&& value) : 
		BaseAPIValue(RTTI_OF(T)), 
		mValue(std::move(value)) {  }


	template<typename T>
	nap::APIValue<T>::APIValue(const T& value) : 
		BaseAPIValue(RTTI_OF(T)), 
		mValue(value) { }


	template<typename T>
	nap::APIValue<T>::APIValue() : BaseAPIValue(RTTI_OF(T)) { }


	template<typename T>
	nap::APIValue<T>::APIValue(const APIValue& other) : 
		BaseAPIValue(RTTI_OF(T)),
		mValue(other.mValue)
	{
	}


	template<typename T>
	nap::APIValue<T>::APIValue(APIValue&& other) : 
		BaseAPIValue(RTTI_OF(T)),
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


	template<typename T>
	T* nap::APIArgument::get()
	{
		return rtti_cast<T>(mAPIValue.get());
	}


	template<typename T>
	const T* nap::APIArgument::get() const
	{
		return rtti_cast<T>(mAPIValue.get());
	}


	template<typename T>
	T nap::APIArgument::getCopy()
	{
		T* val_ptr = this->get<T>();
		assert(val_ptr != nullptr);
		return *val_ptr;
	}


	template<typename T>
	const std::vector<T>* nap::APIArgument::asArray() const
	{
		APIValue<std::vector<T>>* api_value = get<APIValue<std::vector<T>>>();
		if (api_value == nullptr)
			return nullptr;

		// Return array
		return &api_value->mValue;
	}


	template<typename T>
	std::vector<T>* nap::APIArgument::asArray()
	{
		APIValue<std::vector<T>>* api_value = get<APIValue<std::vector<T>>>();
		if (api_value == nullptr)
			return nullptr;

		// Return array
		return &api_value->mValue;
	}
}
