#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <nap/object.h>
#include <nap/modulemanager.h>

namespace nap
{
	using ObjPtr = int64_t;

    /**
     * Retrieve the serializable pointer to an Object
     * @param obj The object to retrieve the address from
     * @return An @see Object or nullptr when the cast failed
     */
	static ObjPtr toPtr(const Object& obj) { return reinterpret_cast<ObjPtr>(&obj); }

    /**
     * Retrieve the serializable pointer to an Object
     * @param obj The object to retreive the address from
     * @return An @see Object or nullptr when the cast failed
     */
	static ObjPtr toPtr(const Object* obj) { return toPtr(*obj); }

    /**
     * PIECE OF SHIT
     * @param attrib Something along the lines of "myType"
     * @return Something along the lines of "nap::Attribute<myType>"
     */
    static std::string dirtyHack(const std::string& attrib) { return "nap::Attribute<" + attrib + ">"; }

    /**
     * Abstract base class for all Object tree serializers
     */
	class Serializer
	{
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
	};


	class Deserializer
	{

	public:
		virtual Object* readObject(std::istream& istream, Core& core, Object* parent = nullptr) const = 0;
		Object* fromString(const std::string& str, Core& core, Object* parent = nullptr) const;
	};
}
