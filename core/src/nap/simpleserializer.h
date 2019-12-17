#include <string>
#include <rttr/type>

namespace nap
{
	/**
	 * Load a json string and deserialize its data onto the given object.
	 *
	 * @param json The json string to read. The object to be deserialized should be represented by the document root.
	 * @param obj The object to store the loaded data on.
	 * @param err Will be populated with any errors when they happen.
	 * @return True on success, false otherwise.
	 */
	bool deserializeSimple(const std::string& json, rttr::instance obj, nap::utility::ErrorState& err);

	/**
	 * Load a json file and deserialize its data onto the given object.
	 *
	 * @param filename The json file to load. The object to be deserialized should be represented by the document root.
	 * @param obj The object to store the loaded data on.
	 * @param err Will be populated with any errors when they happen.
	 * @return True on success, false otherwise.
	 */
	bool loadJSONSimple(const std::string& filename, rttr::instance obj, nap::utility::ErrorState& err);
}
