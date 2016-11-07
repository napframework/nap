#include "objectpath.h"
#include "patch.h"
#include "stringutils.h"
#include "logger.h"

#define DELIMITER "/"
/**
 * ObjectPath represents a string based pointer to an object in the Object tree.
 *
 * The root node is represented by a delimiter "/"
 *
 *
 */
namespace nap
{


	// Walk from the specified object down to the root
	void ObjectPath::constructPath(const Object& node)
	{
		// Store path here, leaf to root, then reverse construct string
		std::vector<std::string> path;
		const Object* current = &node;
		while (current->getParentObject()) {
			path.push_back(current->getName());
			path.push_back(DELIMITER);
			current = current->getParentObject();
		}

		// Now reverse construct the reverse path
		if (path.size() == 0)
			mAbsolutePath = "/";
		else
			mAbsolutePath = pathToString(path);
	}

	const std::string ObjectPath::pathToString(const std::vector<std::string>& path) const
	{
		std::ostringstream ss;
		auto first = path.rbegin();
		auto last = path.rend();

		for (; first != last; ++first) {
			ss << *first;
		}
		return ss.str();
	}

	Object* ObjectPath::resolve(Object& root) const
	{
		if (mAbsolutePath == DELIMITER) return root.getRootObject();

		std::list<std::string> tokens;
		gTokenize(mAbsolutePath, tokens, DELIMITER);

		nap::Object* currentNode = root.getRootObject();

		while (!tokens.empty()) {
			//			std::string delimToken = tokens.front();
			tokens.pop_front(); // Pop delimiter

			if (tokens.empty()) {
				Logger::debug("Path is malformed: %s", mAbsolutePath.c_str());
				return nullptr;
			}

			currentNode = currentNode->getChild(tokens.front());
			if (!currentNode)  {
				Logger::debug("Failed to resolve path: %s", mAbsolutePath.c_str());
				return nullptr;
			}
			tokens.pop_front();
		}
		return currentNode;
	}
    void ObjectPath::clear() { mAbsolutePath.clear(); }
    bool ObjectPath::isEmpty() const { return mAbsolutePath.empty(); }
    ObjectPath& ObjectPath::operator=(const nap::ObjectPath& path) {
        mAbsolutePath = ObjectPath(path.mAbsolutePath);
        return *this;
    }
    ObjectPath::ObjectPath(const Object* object) {
        assert(object);
        constructPath(*object);
    }
    ObjectPath::ObjectPath(const ObjectPath& p) { mAbsolutePath = p.mAbsolutePath; }
    ObjectPath::ObjectPath(const std::string& path) : mAbsolutePath(path) {}
    ObjectPath::ObjectPath(const Object& object) { constructPath(object); }
}