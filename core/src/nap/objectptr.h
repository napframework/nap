#pragma once

#include <unordered_set>
#include <rtti/rttiobject.h>

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
	class ObjectPtrManager
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
	class ObjectPtrBase
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
		friend class ResourceManagerService;

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
		using wrapped_type = decltype(std::declval<nap::ObjectPtr<T>>().get());
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
