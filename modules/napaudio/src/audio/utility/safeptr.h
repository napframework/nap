#pragma once

// Std includes
#include <set>
#include <functional>

// Nap includes
#include <utility/concurrentqueue.h>

namespace nap
{
    
    namespace audio
    {
        
        // Forward declarations
        class DeletionQueue;
        template <typename T> class SafeOwner;
        template <typename T> class SafePtr;

        
        /**
         * The DeletionQueue holds the data that was owned by @SafeOwner smart-pointers until they went out of scope.
         * SafeOwner's destructor disposes it's owned data in the DeletionQueue and the DeletionQueue takes over ownership over this data. The data does not only contain a pointer to the owned object but also a list of all SafePtrs that point to the SafeOwner's data.
         * The DeletionQueue needs to be cleared regularly using the clear() method to free the heap of disposed data. When the DeletionQueue is cleared all SafePtrs that point to an object in the bin are set to nullptr.
         * By making use of the DeletionQueue in combination with @SafeOwner and @SafePtr the programmer can control or restrict the moment where objects are  destructed and also choose the thread on which this will happen.
         */
        class DeletionQueue final
        {
        public:
            DeletionQueue() = default;
            ~DeletionQueue()
            {
                clear();
            }
            
            /**
             * This method is used by @SafeOwner to dispose it's data when it goes out of scope.
             * The disposed data will be kept by the DeletionQueue and will be destructed and freed on the next call of clear().
             */
            template <typename T>
            void enqueue(typename SafeOwner<T>::Data* ownerData)
            {
                mQueue.enqueue([ownerData]()
                {
                    destroy<T>(ownerData);
                });
            }

            /**
             * Clears the DeletionQueue by destructing the objects is contains.
             * It also sets all SafePtrs that point to the objects in the DeletionQueue to nullptr.
             */
            void clear()
            {
                DeleteFunction deleter = nullptr;
                while (mQueue.try_dequeue(deleter))
                {
                    deleter();
                }
            }
            
        private:
            /*
             * Destroys one object from the bin and sets all SafePtrs in the list pointing to the object to nullptr.
             */
            template <typename T>
            static void destroy(typename SafeOwner<T>::Data* ownerData)
            {
                // Sets SafePtrs pointing to the data that will be destroyed to nullptr.
                for (auto& ptr : ownerData->mPointers)
                    ptr->mOwnerData = nullptr;
                // Delete the object
                delete ownerData->mObject;
                // Delete the enclosing struct.
                delete ownerData;
            }
            
            using DeleteFunction = std::function<void()>;
            moodycamel::ConcurrentQueue<DeleteFunction> mQueue; ///< Lockfree queue that holds lambda's that each delete one object in the DeletionQueue. The pointer to the data to be deleted is held in the capture list of the lambda.
        };
        
        
        /**
         * Base class for SafeOwner<T> template
         */
        class SafeOwnerBase
        {
        public:
            virtual ~SafeOwnerBase() = default;
            
        protected:
            virtual void* getData() = 0;
            virtual void setData(void* data) = 0;
            virtual DeletionQueue* getDeletionQueue() = 0;
            virtual void setDeletionQueue(DeletionQueue* queue) = 0;
            virtual void enqueueForDeletion() = 0;

            void assign(SafeOwnerBase& source)
            {
                // When assigning from a different owner we first need to trash the current content (if any)
                enqueueForDeletion();
                
                // Then we copy the source data
                setData(source.getData());
                
                // Because we transfer ownership here we set the source's data pointer to nullptr.
                source.setData(nullptr);
                
                // We have to copy the deletion queue in case this SafeOwner was constructed with nullptr
                setDeletionQueue(source.getDeletionQueue());
            }
        };
        
        
        /**
         * SafeOwner is a special case smart pointer to an object that is used in multiple threads. It serves the purpose of making sure that the object is destructed in a thread safe manner when the SafeOwner goes out of scope.
         * It works very much like unique_ptr in such that it takes ownership over the object it points to and takes responsibility for it's destruction.
         * The difference to unique_ptr is that instead of letting the object destruct itself when the pointer goes out of scope, it throws the object in a DeletionQueue.
         * Objects in the DeletionQueue are kept alive until the trash bin is emptied at a moment when there is noone else using the object anymore.
         * This is done to prevent the objects to be destroyed while they are possibly being used in another thread at the same time.
         */
        template <typename T>
        class SafeOwner final : public SafeOwnerBase
        {
            friend class DeletionQueue;
            friend class SafePtr<T>;
            
        private:
            /**
             * The data that SafeOwner manages does not only consist of the managed object but also contains a list of all SafePtrs that point to the object as well.
             */
            class Data
            {
            public:
                T* mObject = nullptr; // The pointer to the object managed by the SafeOwner internally
                std::set<SafePtr<T>*> mPointers;
            };
            
        public:
            SafeOwner() = default;
            
            /**
             * The constructor takes the DeletionQueue that the owner will use to dispose it's managed object and a pointer to the object it will manage.
             */
            SafeOwner(DeletionQueue& deletionQueue, T* ptr)
            {
                mDeletionQueue = &deletionQueue;
                mData = new Data();
                mData->mObject = ptr;
            }
            
            /**
             * The destructor of the SafeOwner base class. Instead of letting the managed object destruct itself it is thrown into the trash bin, from which it will be deleted on a safe moment.
             */
            ~SafeOwner()
            {
                enqueueForDeletion();
            }
            
            // Copy constructor with nullptr
            SafeOwner(const std::nullptr_t) { }
            
            // Assignment operator with nullptr
            SafeOwner& operator=(const std::nullptr_t)
            {
                enqueueForDeletion();
                return *this;
            }
            
            // Copy and move are not allowed because the SafeOwner (just like unique_ptr) holds ownership
            SafeOwner(const SafeOwner& other) = delete;
            SafeOwner& operator=(const SafeOwner& other) = delete;
            
            // Move ctor is fine, as unique_ptr's can be moved
            SafeOwner(SafeOwner<T>&& other)
            {
                assign(other);
            }
            
            // Move assignment operator
            SafeOwner<T>& operator=(SafeOwner<T>&& other)
            {
                assign(other);
                return *this;
            }
            
            // Move constructor with different type
            template <typename OTHER>
            SafeOwner(SafeOwner<OTHER>&& other)
            {
                assign(other);
            }
            
            // Move assignment operator with different type
            template <typename OTHER>
            SafeOwner<T>& operator=(SafeOwner<OTHER>&& other)
            {
                assign(other);
                return *this;
            }
            
            const T& operator*() const
            {
                assert(mData != nullptr);
                return *static_cast<T*>(mData->mObject);
            }
            
            T& operator*()
            {
                assert(mData != nullptr);
                return *static_cast<T*>(mData->mObject);
            }
            
            T* operator->() const
            {
                assert(mData != nullptr);
                return static_cast<T*>(mData->mObject);
            }
            
            T* operator->()
            {
                assert(mData != nullptr);
                return static_cast<T*>(mData->mObject);
            }
            
            template<typename OTHER>
            bool operator==(const OTHER* ptr) const
            {
                
                return ptr ? mData->mObject == ptr : mData == nullptr;
            }
            
            bool operator==(const std::nullptr_t) const
            {
                
                return mData == nullptr;
            }
            
            template<typename OTHER>
            bool operator!=(const OTHER* ptr) const
            {
                return ptr ? mData->mObject != ptr : mData != nullptr;
            }
            
            /**
             * Returns a SafePtr to the owned object. The SafePtr (and all it's future copies) will be set to nullptr when the managed object will be destroyed by the DeletionQueue.
             */
            SafePtr<T> get()
            {
                auto safePtr = SafePtr<T>(*this);
                return safePtr;
            }
            
            /**
             * Returns a const SafePtr to the owned object. The SafePtr (and all it's future copies) will be set to nullptr when the managed object will be destroyed by the DeletionQueue.
             */
            SafePtr<T> get() const
            {
                auto safePtr = SafePtr<T>(*this);
                return safePtr;
            }
            
            /**
             * Returns a raw pointer to the owned object.
             */
            T* getRaw() { return mData->mObject; }
            
            /**
             * Returns a const raw pointer to the owned object.
             */
            T* getRaw() const { return mData->mObject; }

        protected:
            void* getData() override { return mData; }
            
            void setData(void* data) override { mData = static_cast<Data*>(data); }
            
            DeletionQueue* getDeletionQueue() override { return mDeletionQueue; }
            
            void setDeletionQueue(DeletionQueue* queue) override { mDeletionQueue = queue; }
            
        private:
            /**
             * Disposes the managed object into the DeletionQueue. The DeletionQueue will free the object the next time it is cleared.
             */
            void enqueueForDeletion() override
            {
                if (mData != nullptr)
                {
                    assert(mDeletionQueue != nullptr);
                    mDeletionQueue->enqueue<T>(mData);
                    mData = nullptr;
                }
            }
            
            Data* mData = nullptr; ///< The data containing a pointer to the owned object and pointers to all safe pointers pointing to this
            DeletionQueue* mDeletionQueue = nullptr; ///< Pointer to the DeletionQueue thhis SafeOwner will use to dispose of it's managed object.
        };
        
        
        /**
         * Base class for SafePtr<T> template.
         */
        class SafePtrBase
        {
        public:
            virtual ~SafePtrBase() = default;
            
        protected:
            virtual void* getOwnerData() const = 0;
            virtual void setOwnerData(void* ownerData) = 0;
            
            void assign(const SafePtrBase& other)
            {
                setOwnerData(other.getOwnerData());
            }
        };
        

        /**
         * A SafePtr points to an object that is owned by a @SafeOwner somewhere. The SafePtr will remain valid when the owner has gone out of scope or when it has otherwise trashed the pointed object. Only when the @DeletionQueue is cleared the SafePtr will be set to nullptr and the data it previously pointed to will be rendered invalid.
         */
        template <typename T>
        class SafePtr final : public SafePtrBase
        {
            friend class SafeOwner<T>;
            friend class DeletionQueue;
            
        public:
            SafePtr() = default;
            
            /**
             * Constructs a SafePtr pointing to an object owned by a @SafeOwner.
             */
            SafePtr(SafeOwner<T>& owner)
            {
                mOwnerData = owner.mData;
                mOwnerData->mPointers.emplace(this);
            }
            
            /**
             * The destructor removes this Safeptr from the list of SafePtrs that will be notified when the object is destroyed.
             */
            ~SafePtr()
            {
                if (mOwnerData != nullptr)
                    mOwnerData->mPointers.erase(this);
            }
            
            // Copy ctor
            SafePtr(const SafePtr<T>& other)
            {
                assign(other);
            }
            
            // Assignment operator
            SafePtr& operator=(const SafePtr<T>& other)
            {
                assign(other);
                return *this;
            }
            
            // Copy ctor with different type
            template <typename OTHER>
            SafePtr(const SafePtr<OTHER>& other)
            {
                assign(other);
            }
            
            // Assignment operator with different type
            template <typename OTHER>
            SafePtr& operator=(const SafePtr<OTHER>& other)
            {
                assign(other);
                return *this;
            }
            
            // Copy ctor with nullptr
            SafePtr(const std::nullptr_t) { }
            
            // Assignment operator with nullptr
            SafePtr& operator=(const std::nullptr_t)
            {
                if (mOwnerData != nullptr)
                    mOwnerData->mPointers.erase(this);
                mOwnerData = nullptr;
                return *this;
            }
            
            // Move ctor
            SafePtr(SafePtr<T>&& other)
            {
                assign(other);
            }
            
            // Move assignment operator
            SafePtr<T>& operator=(SafePtr<T>&& other)
            {
                assign(other);
                return *this;
            }
            
            // Move ctor with different type
            template <typename OTHER>
            SafePtr(SafePtr<OTHER>&& other)
            {
                assign(other);
            }
            
            // Move assignment operator with different type
            template <typename OTHER>
            SafePtr<T>& operator=(SafePtr<OTHER>&& other)
            {
                assign(other);
                return *this;
            }
            
            const T& operator*() const
            {
                assert(mOwnerData->mObject != nullptr);
                return *static_cast<T*>(mOwnerData->mObject);
            }
            
            T& operator*()
            {
                assert(mOwnerData != nullptr);
                return *static_cast<T*>(mOwnerData->mObject);
            }
            
            T* operator->() const
            {
                assert(mOwnerData != nullptr);
                return static_cast<T*>(mOwnerData->mObject);
            }
            
            T* operator->()
            {
                assert(mOwnerData != nullptr);
                return static_cast<T*>(mOwnerData->mObject);
            }
            
            template<typename OTHER>
            bool operator==(const OTHER* ptr) const
            {
                return ptr ? mOwnerData->mObject == ptr : mOwnerData == nullptr;
            }
            
            template<typename OTHER>
            bool operator!=(const OTHER* ptr) const
            {
                return ptr ? mOwnerData->mObject != ptr : mOwnerData != nullptr;
            }
            
            bool operator==(const std::nullptr_t) const
            {
                return mOwnerData == nullptr;
            }
            
            bool operator!=(const std::nullptr_t) const
            {
                return mOwnerData != nullptr;
            }
            
            T* get() const
            {
                assert(mOwnerData != nullptr);
                return static_cast<T*>(mOwnerData->mObject);
            }
            
            T* get()
            {
                assert(mOwnerData != nullptr);
                return static_cast<T*>(mOwnerData->mObject);
            }
            
        private:
            void* getOwnerData() const override { return mOwnerData; }
            
            void setOwnerData(void* ownerData) override
            {
                // Remove itself from the list of SafePtrs that point to the previous target object.
                if (mOwnerData != nullptr)
                    mOwnerData->mPointers.erase(this);
                
                // Update the target data
                mOwnerData = static_cast<typename SafeOwner<T>::Data*>(ownerData);
                
                // Register with the list of SafePtrs pointing to the new target
                mOwnerData->mPointers.emplace(this);
            }
            
            typename SafeOwner<T>::Data* mOwnerData = nullptr; ///< The data pointed to by this SafePtr, managed by @SafeOwner or by the @DeletionQueue.
        };
        
        
    }
    
}

