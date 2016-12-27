#pragma once

// Local Includes
#include "object.h"

// External Includes
#include <iostream>
#include <list>

namespace nap
{
	/**
	* ObjectPath represents a string based pointer to an object in the Object tree.
	*
	* The root node is represented by a delimiter "/"
	*/
	class ObjectPath
	{

	public:
		ObjectPath() {}
        ObjectPath(const ObjectPath& p);
        ObjectPath(const std::string& path);
        ObjectPath(const Object* object);
        ObjectPath(const Object& object);

		template <typename T>
		T* resolve(Object& root)
		{
            Object* ob = resolve(root);
            if (nullptr == ob)
                return nullptr;
            if (ob->getTypeInfo().isKindOf<T>())
                return static_cast<T*>(ob);
			return nullptr;
		}

        void clear();
        bool isEmpty() const;

		Object* resolve(Object& root) const;

		const std::string& toString() const { return mAbsolutePath; }

		operator const std::string&() const { return toString(); }
		bool operator==(const std::string& other) const { return toString() == other; }
		bool operator==(const ObjectPath& other) const { return mAbsolutePath == other.mAbsolutePath; }
		bool operator!=(const ObjectPath& other) const { return mAbsolutePath != other.mAbsolutePath; }
		bool operator <(const ObjectPath& other) const { return mAbsolutePath < other.mAbsolutePath; }

        ObjectPath& operator=(const nap::ObjectPath& path);
	private:
		void constructPath(const Object& node);
		const std::string pathToString(const std::vector<std::string>& path) const;

		std::string mAbsolutePath = "";
	};
}