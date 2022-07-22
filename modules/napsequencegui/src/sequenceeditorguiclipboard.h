/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <rtti/rtti.h>
#include <rtti/object.h>
#include <rtti/jsonreader.h>
#include <rtti/defaultlinkresolver.h>
#include <fstream>
#include <rtti/objectptr.h>
#include <nap/logger.h>

namespace nap
{
    namespace sequenceguiclipboard
    {
        //////////////////////////////////////////////////////////////////////////

        /**
         * Clipboard is a class that can contain a serialized object related to the Sequencer
         * The state of the gui can contain a clipboard, that can be de-serialized at a certain point
         * Typically, the clipboard contains a certain curve segment or event segment, and that segment can be pasted
         * at a certain location in a certain Track
         */
        class NAPAPI Clipboard
        {
        RTTI_ENABLE()

        public:
            /**
             * Constructor
             * @param trackType expects tracktype
             */
            explicit Clipboard(const rttr::type& trackType);

            /**
             * Default decontructor
             */
            virtual ~Clipboard() = default;

            /**
             * Serialize an object
             * @param object pointer object to serialize
             * @param sequenceName sequence name currently used, if sequence name is different from previous, clears previously added objects
             * @param errorState holds information about any errors
             */
            void addObject(const rtti::Object* object, const std::string& sequenceName, utility::ErrorState& errorState);

            /**
             * Serialize an object
             * @param object pointer object to serialize
             * @param errorState holds information about any errors
             */
            void addObject(const rtti::Object* object, utility::ErrorState& errorState);

            /**
             * Deserialize clipboard content to object of type T
             * @tparam T the object type to deserialze
             * @param createdObjects vector containing created objects
             * @param errorState holds information about any errors
             * @return pointer to root object, can be null and must be of type T
             */
            template<typename T>
            std::vector<T*> deserialize(std::vector<std::unique_ptr<rtti::Object>>& createdObjects, utility::ErrorState& errorState);


            /**
             * returns true when clipboard is clipboard of derived type T
             * @tparam T the derived type
             * @return true when T is of derived type
             */
            template<typename T>
            bool isClipboard()
            {
                return this->get_type() == RTTI_OF(T);
            }


            /**
             * returns raw pointer to derived class T of this clipboard
             * performs static cast, exception on fail, always use isClipboard<T> to check for type
             * @tparam T the derived type
             * @return pointer to T
             */
            template<typename T>
            T* getDerived()
            {
                assert(isClipboard<T>());
                return static_cast<T*>(this);
            }


            /**
             * returns object ids of serialized objects
             * @return vector of object ids
             */
            std::vector<std::string> getObjectIDs() const;

            /**
             * returns true when clipboard holds serialized object of specified id
             * @param objectID the object id of serialized object
             * @return true if clipboard contains serialized object
             */
            bool containsObject(const std::string& objectID, const std::string& sequenceName) const;

            /**
             * removes specified object from clipboard when it is contained, no assert when not present
             * @param objectID the object id of the object to remove
             */
            void removeObject(const std::string& objectID);

            /**
             * returns amount of stored serialized objects
             * @return amount of stored serialized objects
             */
            int getObjectCount() const{ return mSerializedObjects.size(); }

            /**
             * returns rtti track type info that was passed to clipboard upon construction
             * @return rtti track type info
             */
            rttr::type getTrackType() const{ return mTrackType; }

            /**
             * Writes current serialized segments to disk, return true on success, errorState contains any errors
             * FilePath is relative to application data folder
             * @param filePath path of file to write relative to application data folder
             * @param errorState contains any errors that may occur during this operation
             * @return true on success
             */
            bool save(const std::string& filePath, utility::ErrorState& errorState);

            /**
             * Loads serialized segments to clipboard, return true on success, errorState contains any errors
             * @param filePath path of file to load relative to application data folder
             * @param errorState contains any errors that may occur during this operation
             * @return true on success
             */
            bool load(const std::string& filePath, utility::ErrorState& errorState);

        protected:
            // the serialized objects
            std::map<std::string, std::string> mSerializedObjects;
            rttr::type mTrackType;
            std::string mSequenceName;
        };

        // shortcut
        using SequenceClipboardPtr = std::unique_ptr<Clipboard>;


        // use this method to create a clipboard
        template<typename T, typename ... Args>
        static SequenceClipboardPtr createClipboard(Args &&... args)
        {
            return std::make_unique<T>(std::forward<Args>(args)...);
        }


        /**
         * Empty clipboard
         */
        class NAPAPI Empty : public Clipboard
        {
        RTTI_ENABLE()
        public:
            Empty() : Clipboard(rttr::type::empty())
            {}
        };


        //////////////////////////////////////////////////////////////////////////
        // Template definitions
        //////////////////////////////////////////////////////////////////////////

        template<typename T>
        std::vector<T*> Clipboard::deserialize(std::vector<std::unique_ptr<rtti::Object>>& createdObjects, utility::ErrorState& errorState)
        {
            std::vector<T *> deserialized_objects;

            createdObjects.clear();
            for(auto &pair: mSerializedObjects)
            {
                // get the serialized object from the map
                std::string serialized_object = pair.second;

                // De-serialize
                rtti::DeserializeResult result;
                rtti::Factory factory;
                if(!rtti::deserializeJSON(serialized_object, rtti::EPropertyValidationMode::DisallowMissingProperties,
                                          rtti::EPointerPropertyMode::NoRawPointers, factory, result, errorState))
                    return std::vector<T *>();

                // Resolve links
                if(!rtti::DefaultLinkResolver::sResolveLinks(result.mReadObjects, result.mUnresolvedPointers, errorState))
                    return std::vector<T *>();

                // Make sure we deserialized something
                T *root_object = nullptr;
                if(!errorState.check(!result.mReadObjects.empty(), "No objects deserialized"))
                    return std::vector<T *>();

                auto *first_object = result.mReadObjects[0].get();
                if(!errorState.check(first_object->get_type() == RTTI_OF(T) ||
                                     first_object->get_type().is_derived_from<T>(), "Root object not of correct type"))
                    return std::vector<T *>();

                // Move ownership of read objects
                root_object = static_cast<T *>(first_object);
                for(auto &read_object: result.mReadObjects)
                {
                    createdObjects.emplace_back(std::move(read_object));
                }

                // init objects
                for(auto &object_ptr: createdObjects)
                {
                    if(!errorState.check(object_ptr->init(errorState), "Error initializing object : %s ", errorState.toString().c_str()))
                        return std::vector<T *>();
                }

                if(!errorState.check(root_object != nullptr, "return object is null"))
                    return std::vector<T *>();

                deserialized_objects.emplace_back(root_object);
            }

            return deserialized_objects;
        }
    }
}
