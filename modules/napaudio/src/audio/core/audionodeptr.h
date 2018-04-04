#pragma once

// Nap include
#include <nap/logger.h>

// Audio includes
#include <audio/core/audionodemanager.h>
#include <audio/core/audionode.h>

namespace nap
{
    
    namespace audio
    {
        
        // Forward declarations
        template <typename T>
        class NodePtr;
        
        
        /**
         * This function works like std::make_unique() but instead constructs and returns a smart NodePtr.
         * NodePtr basically works like unique_ptr but instead of destructing the Node when going out of scope the Node will be left alive until the next audio callback, when it will be destructed thread-safely.
         */
        template <typename T, typename... Args>
        NodePtr<T> make_node(Args&&... args)
        {
            return NodePtr<T>(new T(std::forward<Args>(args)...));
        }
        
        
        /*
         * Base class for all NodePtr types. Because it is a friend class of NodeManager it can dispose the internally managed unique_ptr in the NodeManager's trash bin.
         * NodePtrs basically work like unique_ptr but instead of destructing the Node when going out of scope the Node will be left alive untill the next audio callback, when it will be destructed thread-safely.
         */
        class NodePtrBase
        {
        public:
            NodePtrBase() = default;
            
            // Copy and move are not allowed because it would copy a unique_tr
            NodePtrBase(const NodePtrBase& other) = delete;
            NodePtrBase& operator=(const NodePtrBase& other) = delete;
            
            /*
             * The destructor of the NodePtr base class. Instead of letting the managed unique_ptr<Node> destruct itself it is thrown into the NodeManager's trash bin, from which it will be deleted on the next audio callback.
             */
            virtual ~NodePtrBase()
            {
                if (mPtr != nullptr)
                    mPtr->getNodeManager().mNodeTrashBin.enqueue(std::move(mPtr));
            }
            

        protected:
            template <typename OTHER>
            void assign(NodePtr<OTHER>& other)
            {
                mPtr = std::move(other.mPtr);
                other.mPtr = nullptr;
            }
            
            std::unique_ptr<Node> mPtr = nullptr; // The unique_ptr managed by the NodePtr internally
        };
        
        
        /**
         * NodePtr is a special case smart pointer to an audio node. It serves the purpose to make sure the node is destructed in a thread safe manner when the NodePtr goes out of scope.
         * It works very much like unique_ptr in such that it takes ownership over the node it points to and takes responsibility for it's destruction.
         * The difference to unique_ptr is that instead of letting the object it points to destruct itself when the pointer goes out of scope, it throws the object in the NodeManager's trash bin.
         * Objects in the NodeManager's trash bin are kept alive until the next audio callback, after which they are safely destroyed.
         * This is done to prevent the nodes to be destroyed while they are possibly being processed from the audio thread at the same tim.
         */
        template <typename T>
        class NodePtr : public NodePtrBase
        {
        public:
            NodePtr() = default;
            
            // Regular ptr Ctor
            NodePtr(T* ptr)
            {
                mPtr = std::unique_ptr<T>(ptr);
            }
            
            // Move ctor is fine, as unique_ptr's can be moved
            NodePtr(NodePtr<T>&& other)
            {
                assign(other);
            }
            
            // Move assignment operator
            NodePtr<T>& operator=(NodePtr<T>&& other)
            {
                assign(other);
                return *this;
            }
            
            // Move ctor is fine, as unique_ptr's can be moved
            template <typename OTHER>
            NodePtr(NodePtr<OTHER>&& other)
            {
                assign(other);
            }
            
            // Move assignment operator
            template <typename OTHER>
            NodePtr<T>& operator=(NodePtr<OTHER>&& other)
            {
                assign(other);
                return *this;
            }
            
            const T& operator*() const
            {
                assert(mPtr != nullptr);
                return *static_cast<T*>(mPtr.get());
            }
            
            T& operator*()
            {
                assert(mPtr != nullptr);
                return *static_cast<T*>(mPtr.get());
            }
            
            T* operator->() const
            {
                assert(mPtr != nullptr);
                return static_cast<T*>(mPtr.get());
            }
            
            T* operator->()
            {
                assert(mPtr != nullptr);
                return static_cast<T*>(mPtr.get());
            }
            
            template<typename OTHER>
            bool operator==(const OTHER* ptr) const
            {
                return mPtr.get() == ptr;
            }
            
            template<typename OTHER>
            bool operator!=(const OTHER* ptr) const
            {
                return mPtr.get() != ptr;
            }
            
            T* get() const
            {
                return static_cast<T*>(mPtr.get());
            }
            
            T* get()
            {
                return static_cast<T*>(mPtr.get());
            }
            
        };
        
    }
    
}
