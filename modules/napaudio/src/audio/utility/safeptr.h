#pragma once

// Std includes
#include <set>
#include <functional>
#include <atomic>

// Nap includes
#include <utility/dllexport.h>
#include <concurrentqueue.h>

namespace nap
{
    
    namespace audio
    {        
        // Forward declarations
        class DeletionQueue;
        template <typename T> class SafeOwner;
        template <typename T> class SafePtr;

        /**
         * Base class for SafeOwner<T> template
         */
        class NAPAPI SafeOwnerBase
        {
            friend class DeletionQueue;

        protected:
            class Data
            {
            public:
                virtual ~Data() = default;
            };

        public:
            virtual ~SafeOwnerBase() = default;

        protected:
            /**
             * Transfer ownership of the object in @source to this. Source will be pointing to nothing.
             * If we are currently holding any object it will be deleted safely using the DeletionQueue.
             */
            void assign(SafeOwnerBase& source);

        private:
            // These methods are internally used by @assign() and will be overwritted by SafeOwner<T> to support polymorphism.
            virtual void* getData() = 0;
            virtual void setData(void* data) = 0;
            virtual DeletionQueue* getDeletionQueue() = 0;
            virtual void setDeletionQueue(DeletionQueue* queue) = 0;
            virtual void enqueueForDeletion() = 0;
        };


        /**
         * The DeletionQueue holds the data that was previously owned by @SafeOwner smart-pointers before they went out of scope.
         * SafeOwner's destructor disposes it's owned data in the DeletionQueue and the DeletionQueue takes over ownership over this data. The data does not only contain a pointer to the owned object but also a list of all SafePtrs that point to the object.
         * The DeletionQueue needs to be cleared regularly using the clear() method to free the heap of disposed data. When the DeletionQueue is cleared all SafePtrs that point to an object held by the queue will be cleared as well.
         * By making use of the DeletionQueue in combination with @SafeOwner and @SafePtr the programmer can control or restrict the moment where objects are destructed and also choose the thread on which this will happen.
         */
        class NAPAPI DeletionQueue final
        {
            friend class SafeOwnerBase;
        public:
            DeletionQueue() = default;
            
            /**
             * The destructor enqueues all existing SafeOwners for deletion and clears the queue so all memory still held will be freed.
             */
            ~DeletionQueue();

            /**
             * This method is used by @SafeOwner to register itself with the queue on construction.
             * This is needed so all existing SafeOwners can be enqueued for deletion when the queue itself is being destructed.
             */
            void registerSafeOwner(SafeOwnerBase* ptr);

            /**
             * This method is used by @SafeOwner to unregister itself with the queue on destruction.
             * This is needed so all existing SafeOwners can be enqueued for deletion when the queue itself is being destructed.
             */
            void unregisterSafeOwner(SafeOwnerBase* ptr);

            /**
             * This method is used by @SafeOwner to dispose it's data when it goes out of scope.
             * The disposed data will be kept by the DeletionQueue and will be destructed and freed on the next call of clear().
             */
            void enqueue(std::unique_ptr<SafeOwnerBase::Data> ownerData)
            {
                mQueue.enqueue(std::move(ownerData));
            }

            void enqueueAll();

            /**
             * Clears the DeletionQueue by destructing the objects it contains.
             * It also clears all the SafePtrs that point to objects in the queue.
             */
            void clear();

        private:
            moodycamel::ConcurrentQueue<std::unique_ptr<SafeOwnerBase::Data>> mQueue; // Lockfree queue that holds lambda's that each delete one object in the DeletionQueue. The pointer to the data to be deleted is held in the capture list of the lambda.
            std::set<SafeOwnerBase*> mSafeOwnerList; // List of all safe owners that exist that are referencing this deletion queue.
        };
        
         
        /**
         * SafeOwner is a special case smart pointer to an object that is used in multiple threads. It serves the purpose of making sure that the object is destructed in a thread safe manner when the SafeOwner goes out of scope.
         * It works very much like unique_ptr in such that it takes ownership over the object it points to and takes responsibility for it's destruction.
         * The difference to unique_ptr is that instead of letting the object destruct itself when the pointer goes out of scope, it throws the object in a DeletionQueue.
         * Objects in the DeletionQueue are kept alive until the DeletionQueue is cleared at a moment when there is noone else using the object anymore.
         * This is done to prevent the objects to be destroyed while they are possibly being used in another thread at the same time.
         */
        template <typename T>
        class SafeOwner final : public SafeOwnerBase
        {
            friend class DeletionQueue;
            friend class SafePtr<T>;
            
        private:
            /**
             * The data that SafeOwner manages does not only consist of the managed object but also contains a list of all SafePtrs that point to the object.
             */
            class Data : public SafeOwnerBase::Data
            {
            public:
                Data(T* ptr)
                {
                    mObject = std::unique_ptr<T>(ptr);
                    mSharedSafePtr = std::make_shared<Data*>(this);
                }

                ~Data() override
                {
                    *mSharedSafePtr = nullptr;
                }

            public:
                std::unique_ptr<T> mObject = nullptr; ///< The pointer to the object managed by the SafeOwner internally.
                std::shared_ptr<Data*> mSharedSafePtr = nullptr;
                std::atomic<bool> mEnqueuedForDeletion = { false }; ///< Indicates wether the object has moved into the deletion queue.
            };
            
        public:
            SafeOwner() = default;
            
            /**
             * The constructor takes the DeletionQueue that the owner will use to dispose it's managed object and a pointer to the object it will manage.
             */
            SafeOwner(DeletionQueue& deletionQueue, T* ptr)
            {
                mDeletionQueue = &deletionQueue;
                mData = std::make_unique<Data>(ptr);
                mDeletionQueue->registerSafeOwner(this);
            }
            
            /**
             * The destructor of the SafeOwner base class. Instead of letting the managed object destruct itself it is thrown into the @DeletionQueue, from which it will be deleted on a safe moment.
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
                return *static_cast<T*>(mData->mObject.get());
            }
            
            T& operator*()
            {
                assert(mData != nullptr);
                return *static_cast<T*>(mData->mObject.get());
            }
            
            T* operator->() const
            {
                assert(mData != nullptr);
                return static_cast<T*>(mData->mObject.get());
            }
            
            T* operator->()
            {
                assert(mData != nullptr);
                return static_cast<T*>(mData->mObject.get());
            }
            
            template<typename OTHER>
            bool operator==(const OTHER* ptr) const
            {
                
                return ptr ? mData->mObject.get() == ptr : mData == nullptr;
            }
            
            template<typename OTHER>
            bool operator!=(const OTHER* ptr) const
            {
                return ptr ? mData->mObject.get() != ptr : mData != nullptr;
            }
            
            bool operator==(const std::nullptr_t) const
            {
                
                return mData == nullptr;
            }
            
            bool operator!=(const std::nullptr_t) const
            {
                
                return mData != nullptr;
            }
            
            /**
             * Returns a SafePtr to the owned object. The SafePtr (and all it's future copies) will be set to nullptr when the managed object will be destroyed by the DeletionQueue.
             */
            SafePtr<T> get()
            {
                auto safePtr = SafePtr<T>(*this);
                return std::move(safePtr);
            }
            
            /**
             * Returns a const SafePtr to the owned object. The SafePtr (and all it's future copies) will be set to nullptr when the managed object will be destroyed by the DeletionQueue.
             */
            SafePtr<T> get() const
            {
                auto safePtr = SafePtr<T>(*this);
                return std::move(safePtr);
            }
            
            /**
             * Returns a raw pointer to the owned object.
             */
            T* getRaw() { return mData->mObject.get(); }
            
            /**
             * Returns a const raw pointer to the owned object.
             */
            T* getRaw() const { return mData->mObject.get(); }

        protected:
            // Inherited from SafeOwnerBase for polymorphism support
            void* getData() override { return mData.release(); }
            void setData(void* data) override { mData = std::unique_ptr<Data>(static_cast<Data*>(data)); }
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
                    mData->mEnqueuedForDeletion = true;
                    mDeletionQueue->enqueue(std::move(mData));
					mDeletionQueue->unregisterSafeOwner(this);
					mDeletionQueue = nullptr;
                    mData = nullptr;
                }
            }
            
            std::unique_ptr<Data> mData = nullptr; ///< The data containing a pointer to the owned object and pointers to all safe pointers pointing to this
            DeletionQueue* mDeletionQueue = nullptr; ///< Pointer to the DeletionQueue this SafeOwner will use to dispose of it's managed object.
        };
        
        
        /**
         * Base class for SafePtr<T> template.
         */
        class NAPAPI SafePtrBase
        {
        public:
            virtual ~SafePtrBase() = default;
            
        protected:
            void assign(const SafePtrBase& other)
            {
                setOwnerData(other.getOwnerData());
            }
            
        private:
            // These methods are internally used by @assign() in order to support polymorphism.
            virtual void* getOwnerData() const = 0;
            virtual void setOwnerData(void* ownerData) = 0;
        };
        

        /**
         * A SafePtr points to an object that is owned by a @SafeOwner somewhere. When the owner goes out of scope and the pointed object will be moved into the @DeletionQueue the SafePtr will return true when checked if it equals nullptr. However the object it points to can still be used and safely accessed using the * and -> operators and the @get() method until the next time the @DeletionQueue is cleared. This way SafePtr guarantees that it can be safely used on both the thread where the SafeOwner went out of scope AND the thread that periodically empties the DeletionQueue, as long as you check if the SafePtr != nullptr before use.
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
                mOwnerData = owner.mData->mSharedSafePtr;
            }
            
            /**
             * The destructor removes this Safeptr from the list of SafePtrs that will be notified when the object is destroyed.
             */
            ~SafePtr()
            {
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
                assert(isValid());
                return *static_cast<T*>((*mOwnerData)->mObject.get());
            }
            
            T& operator*()
            {
                assert(isValid());
                return *static_cast<T*>((*mOwnerData)->mObject.get());
            }
            
            T* operator->() const
            {
                assert(isValid());
                return static_cast<T*>((*mOwnerData)->mObject.get());
            }
            
            T* operator->()
            {
                assert(isValid());
                return static_cast<T*>((*mOwnerData)->mObject.get());
            }
            
            template<typename OTHER>
            bool operator==(const OTHER* ptr) const
            {
                return ptr ? isValid() && (*mOwnerData)->mObject.get() == ptr : !isValid();
            }
            
            template<typename OTHER>
            bool operator!=(const OTHER* ptr) const
            {
                return ptr ? isValid() && (*mOwnerData)->mObject.get() != ptr : !isValid();
            }
            
            bool operator==(const std::nullptr_t) const
            {
                return !isValid() || (*mOwnerData)->mEnqueuedForDeletion == true;
            }
            
            bool operator!=(const std::nullptr_t) const
            {
                return isValid() && (*mOwnerData)->mEnqueuedForDeletion == false;
            }
            
            T* get() const
            {
                assert(isValid());
                return static_cast<T*>((*mOwnerData)->mObject.get());
            }
            
            T* get()
            {
                assert(isValid());
                return static_cast<T*>((*mOwnerData)->mObject.get());
            }
            
        private:
            void* getOwnerData() const override { return *mOwnerData; }
            
            void setOwnerData(void* ownerData) override
            {
                if (ownerData == nullptr)
                    mOwnerData = nullptr;
                else
                    // Update the target data
                    mOwnerData = static_cast<typename SafeOwner<T>::Data*>(ownerData)->mSharedSafePtr;
            }

            bool isValid() const
            {
                return (mOwnerData != nullptr) && (*mOwnerData != nullptr);
            }
            
            std::shared_ptr<typename SafeOwner<T>::Data*> mOwnerData = nullptr; ///< The data pointed to by this SafePtr, managed by @SafeOwner or by the @DeletionQueue.
        };
        
        
    }
    
}

