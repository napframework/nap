#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <nap/modulemanager.h>
#include <nap/object.h>



namespace nap
{
	using ObjPtr = int64_t;


	/**
	 * Abstract base class for all Object tree serializers
	 */
	class Serializer {
    RTTI_ENABLE()
    public:
        /**
         * Write a complete module information
         *
         * @param ostream The stream to write to
         * @param moduleManager The @see ModuleManager to take the information from
         */
        virtual void writeModuleInfo(std::ostream& ostream, ModuleManager& moduleManager) const = 0;

        /**
         * Write an object tree to the stream
         *
         * @param ostream The stream to write to
         * @param object Write from this Object downwards
         * @param writePointers Wherether pointer values should be written too, used for RPC
         */
        virtual void writeObject(std::ostream& ostream, Object& object, bool writePointers = false) const = 0;

        /**
         * Write an Object tree to a string
         *
         * @param object Write from this Object downwards
         * @param writePointers Wherether pointer values should be written too, used for RPC
         * @return A string representing the serialized Object tree
         */
        std::string toString(Object& object, bool writePointers = false) const;

        /**
         * Write a complete module information to a string
         *
         * @param moduleManager The modulemanager to take the information from
         * @return A string representing the complete serialized Object tree
         */
        std::string toString(ModuleManager& moduleManager) const;

        /**
         * Read an object from an input stream. Optionally provide a parent to load the data into. Return nullptr on
         * faillure.
         * @param istream The stream to read from
         * @param core The core to load the data into
         * @param parent The parent object to put the loaded data under
         * @return The loaded Object or nullptr when loading fails
         */
        virtual Object* readObject(std::istream& istream, Core& core, Object* parent = nullptr) const = 0;

        /**
         * Convert a serialized string into a an object.
         * @param str
         * @param core The core in which the deserialized Object is placed
         * @param parent The parent object to put the loaded data under
         * @return A pointer to the loaded Object or nullptr when loading failed
         */
        Object* fromString(const std::string& str, Core& core, Object* parent = nullptr) const;

        /**
         * Read a file and deserialize the object tree in it.
         * @param filename The relative or absolute filename to be loaded
         * @param core The core in which the deserialized Object is placed
         * @param parent The parent object to put the loaded data under
         * @return A pointer to the loaded Object or nullptr when loading failed
         */
        Object* load(const std::string& filename, Core& core, Object* parent = nullptr) const;

        /**
         * Retrieve the serializable pointer to an Object
         * TODO: Make safe
         * @param obj The object to retrieve the address from
         * @return An Object or nullptr when the cast failed
         */
        static ObjPtr toPtr(const Object& obj) { return reinterpret_cast<ObjPtr>(&obj); }

        /**
         * Retrieve the serializable pointer to an Object
         * @param obj The object to retreive the address from
         * @return An Object or nullptr when the cast failed
         */
        static ObjPtr toPtr(const Object* obj) { return toPtr(*obj); }

	};
}

RTTI_DECLARE_BASE(nap::Serializer)
