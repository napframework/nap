#pragma once

// Local Includes
#include "utility/dllexport.h"

// External Includes
#include <unordered_set>
#include <rtti/rttiobject.h>
#include <pybind11/cast.h>

namespace nap
{

	class ObjectPtrBase;
	
	/**
	 * Holds a set of all ObjectPtrs in the application. The purpose of the manager is to be able to
	 * retarget ObjectPtrs if objects get replaced by another object in the real-time updating system.
	 *
	 * The way this is done is by storing *pointers to ObjectPtrs*. The reason is that this makes it 
	 * possible to alter the contents of the pointer at any time without introducing an extra 
	 * indirection: the ObjectPtr just behaves as a regular pointer.
	 * Because we are storing pointers, we need to make sure that at any time the ObjectPtr moves
	 * in memory, we should remove/add the ObjectPtr from the manager. This is done by the ObjectPtr
	 * on every possible occasion where they are moved from on location in memory to another.
	 * 
	 * One possible thing to note is that when objects get destructed, but the destructor isn't called
	 * (which is obviously incorrect), this may cause dangling pointers in this manager. The only case
	 * where this may happen is when destructors aren't virtual and the derived class holds ObjectPtrs.
	 */
	class NAPAPI ObjectPtrManager
	{
	public:
		typedef std::unordered_set<ObjectPtrBase*> ObjectPtrSet;

		/**
		* Returns the global ObjectPtrManager.
		*/
		static ObjectPtrManager& get();


		/**
		* @return the set of ObjectPtrs in the system.
		*/
		ObjectPtrSet& GetObjectPointers()
		{
			return mObjectPointers;
		}

	private:
		template<class T> friend class ObjectPtr;

		/**
		 * Adds a pointer to the manager.
		 */
		void add(ObjectPtrBase& ptr)
		{
			mObjectPointers.insert(&ptr);
		}

		/**
 		 * Removes a pointer from the manager.
 		 */
		void remove(ObjectPtrBase& ptr)
		{
			mObjectPointers.erase(&ptr);
		}

		ObjectPtrSet mObjectPointers;		///< Set of all pointers in the manager
	};


	/**
	 * Abstract class that contains storage for an RTTIObject pointer. This separation is necessary
	 * so that ObjectPtrManager can contain a set of pointers with a known base type (RTTIObject), while the 
	 * clients use derived pointers that are strongly typed.
	 */
	class NAPAPI ObjectPtrBase
	{
	private:
		ObjectPtrBase() = default;

		/**
		* ctor taking direct pointer.
		*/
		ObjectPtrBase(rtti::RTTIObject* ptr) :
			mPtr(ptr)
		{
		}

		/**
		* @return RTTIObject pointer.
		*/
		rtti::RTTIObject* get()
		{
			return mPtr;
		}

		/**
		* @return RTTIObject pointer.
		*/
		const rtti::RTTIObject* get() const
		{
			return mPtr;
		}

		/**
		* @param ptr new pointer to set.
		*/
		void set(rtti::RTTIObject* ptr)
		{
			mPtr = ptr;
		}
	private:
		template<class T> friend class ObjectPtr;
		friend class ResourceManager;

		rtti::RTTIObject* mPtr = nullptr;
	};


	/**
	 * Acts like a regular pointer. Accessing the pointer does not have different performance characteristics than accessing a regular
	 * pointer. Moving/copying an ObjectPtr has a small overhead, as it removes/adds itself from the ObjectPtrManager in such cases.
	 * The purpose of ObjectPtr is that the internal pointer can be changed by the system, so it is not allowed to store pointers or 
	 * references to the internal pointer, as it may get replaced (and destructed) by the system.
	 */
	template<typename T>
	class ObjectPtr : public ObjectPtrBase
	{
	public:
		ObjectPtr() = default;

		// Regular ptr Ctor
		ObjectPtr(T* ptr) :
			ObjectPtrBase(ptr)
		{
			if (mPtr != nullptr)
				ObjectPtrManager::get().add(*this);
		}

		// Copy ctor
		ObjectPtr(const ObjectPtr<T>& other)
		{
			Assign(other);
		}

		// Move ctor
		ObjectPtr(ObjectPtr<T>&& other)
		{
			Assign(other);
			other.mPtr = nullptr;
		}

		// Assignment operator
		ObjectPtr<T>& operator=(const ObjectPtr<T>& other)
		{
			Assign(other);
			return *this;
		}

		// Move assignment operator
		ObjectPtr<T>& operator=(ObjectPtr<T>&& other)
		{
			Assign(other);
			other.mPtr = nullptr;
			return *this;
		}

		// Dtor
		~ObjectPtr()
		{
			ObjectPtrManager::get().remove(*this);
		}

		//////////////////////////////////////////////////////////////////////////

		// Regular ctor taking different type
		template<typename OTHER>
		ObjectPtr(const ObjectPtr<OTHER>& other)
		{
			Assign(other);
		}

		// Regular move ctor taking different type
		template<typename OTHER>
		ObjectPtr(ObjectPtr<OTHER>&& other)
		{
			Assign(other);
			other.mPtr = nullptr;
		}

		// Assignment operator taking different type
		template<typename OTHER>
		ObjectPtr<T>& operator=(const ObjectPtr<OTHER>& other)
		{
			Assign(other);
			return *this;
		}

		// Move assignment operator taking different type
		template<typename OTHER>
		ObjectPtr<T>& operator=(ObjectPtr<OTHER>&& other)
		{
			Assign(other);
			other.mPtr = nullptr;
			return *this;
		}

		//////////////////////////////////////////////////////////////////////////

		const T& operator*() const
		{
			assert(mPtr != nullptr);
			return *static_cast<T*>(mPtr);
		}

		T& operator*()
		{
			assert(mPtr != nullptr);
			return *static_cast<T*>(mPtr);
		}

		T* operator->() const
		{
			assert(mPtr != nullptr);
			return static_cast<T*>(mPtr);
		}

		T* operator->()
		{
			assert(mPtr != nullptr);
			return static_cast<T*>(mPtr);
		}

		bool operator==(const ObjectPtr<T>& other) const
		{
			return mPtr == other.mPtr;
		}

		template<typename OTHER>
		bool operator==(const ObjectPtr<OTHER>& other) const
		{
			return mPtr == other.mPtr;
		}

		template<typename OTHER>
		bool operator==(const OTHER* ptr) const
		{
			return mPtr == ptr;
		}

		bool operator!=(const ObjectPtr<T>& other) const
		{
			return mPtr != other.mPtr;
		}

		template<typename OTHER>
		bool operator!=(const ObjectPtr<OTHER>& other) const
		{
			return mPtr != other.mPtr;
		}

		template<typename OTHER>
		bool operator!=(const OTHER* ptr) const
		{
			return mPtr != ptr;
		}

		bool operator<(const ObjectPtr<T>& other) const
		{
			return mPtr < other.mPtr;
		}

		bool operator>(const ObjectPtr<T>& other) const
		{
			return mPtr > other.mPtr;
		}

		bool operator<=(const ObjectPtr<T>& other) const
		{
			return mPtr <= other.mPtr;
		}

		bool operator>=(const ObjectPtr<T>& other) const
		{
			return mPtr >= other.mPtr;
		}

		T* get() const
		{
			return static_cast<T*>(mPtr);
		}

		T* get()
		{
			return static_cast<T*>(mPtr);
		}

	private:
		
		/**
		 * Removes/adds itself from the manager and assigns mPtr.
		 */
		template<typename OTHER>
		void Assign(const ObjectPtr<OTHER>& other)
		{
			if (mPtr == nullptr && other.mPtr != nullptr)
				ObjectPtrManager::get().add(*this);

			if (other.mPtr != mPtr)
				mPtr = static_cast<T*>(other.get());

			if (mPtr == nullptr)
				ObjectPtrManager::get().remove(*this);
		}
	};
}


/**
 * The following construct is required to support ObjectPtr in RTTR as a regular pointer.
 */
namespace rttr
{
	template<typename T>
	struct wrapper_mapper<nap::ObjectPtr<T>>
	{
		using wrapped_type = T*;
		using type = nap::ObjectPtr<T>;
		
		inline static wrapped_type get(const type& obj)
		{
			return obj.get();
		}

		inline static type create(const wrapped_type& value)
		{
			return nap::ObjectPtr<T>(value);
		}		
	};
}

/**
 * There is a problem in pybind11 where ownership of smart pointers is always treated like 'owned by pybind11'.
 * The following code works around this problem. First, some explanation on the problem:
 *
 * Pybind deals with std::unique_ptr and std::shared_ptr correctly. For smart pointers, pybind uses the internal
 * wrapped type and stores that internally. So, a std::unique_ptr<Foo> return type will first unwrap the inner type,
 * which is Foo*. To ensure that the object is scoped properly, the inner object is again stored in an std::unique_ptr<Foo>. 
 * Those unique_ptr constructs are unrelated to the fact that our return type is std::unique_ptr: any object that is 
 * returned that is fully owned by pybind is stored within an std::unique_ptr. The effect of this construct is that
 * unique_ptrs correctly transfer their ownership to another unique_ptr, which is what you would expect when returning
 * a unique_ptr. When the internal unique_ptr goes out of scope, the object is destroyed, which is what you'd expect.
 * 
 * Any function that is returning a std::shared_ptr<Foo> should of course behave differently. Internally, the same 
 * happens though: a std::unique_ptr<Foo> is stored and Foo* is destroyed afterwards. To counter this behaviour, pybind added
 * a template argument to determine how you want your type to be stored. If we pass std::shared_ptr, internally
 * we will instead store a std::shared_ptr. This solves the problem only partially, because the type is first unwrapped and then
 * again inserted into new shared_ptr. The refcounts of the shared_ptrs are unrelated, causing double deletes.
 * 
 * To again counter this problem you can derive from std::shared_from_this. This causes refcounts to be stored inside your class
 * itself. So, it doesn't matter if separate shared_ptrs are unrelated, if they point to the same object, the refcount is fine
 * and no double deletes occur.
 *
 * As a sidenote: the construct where you have to specify how something is returned is stored on the class level. This makes little
 * sense, as it should instead be a property of the returning function: some functions could return by unique_ptr and some by
 * shared_ptr. You cannot decide on the class level how your type is returned.
 *
 * In any case, for smart pointers other than unique_ptr and shared_ptr, we're in a bit of a pickle. We want our types to be 
 * referenced instead of owned. We also don't want to try to hack around on the class level, which would make the class design
 * inflexible.
 * We've found an internal pybind class that we could specialize and alter the behaviour of the ownership for our type. Two things
 * to note:
 *		1) We pass return_value_policy::reference (instead of the default ownership)
 *		2) We pass a nullptr to the cast function for the 'existing_holder' function. The existing holder could already be specified
 *		   as a unique_ptr (as described above, through the class template type), causing it to be stored as a unique_ptr anyway.
 *		   Instead we pass nullptr to circumvent this behaviour.
 */
namespace pybind11 
{
	namespace detail 
	{
		template <typename type>
		struct always_construct_holder<nap::ObjectPtr<type>> : always_construct_holder<void, true>
		{
		};

		template <typename type> class type_caster<nap::ObjectPtr<type>> : public copyable_holder_caster<type, nap::ObjectPtr<type>>
		{
		public:
			static handle cast(const nap::ObjectPtr<type>& src, return_value_policy, handle)
			{
				const auto *ptr = src.get();

				auto st = copyable_holder_caster<type, nap::ObjectPtr<type>>::src_and_type(ptr);

				return type_caster_generic::cast(
					st.first, return_value_policy::reference, {}, st.second,
					nullptr, nullptr, nullptr);
			}
		};
	}
}