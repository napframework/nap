#pragma once

#include <nap/resourcemanager.h>
#include <nap/serializer.h>

namespace nap
{
	/**
	 * Serializer that will read from and write to JSON format
	 */
	class JSONSerializer : public Serializer
	{
		RTTI_ENABLE_DERIVED_FROM(Serializer)
	public:
		void writeObject(std::ostream& ostream, Object& object, bool writePointers = false) const override;
		void writeModuleInfo(std::ostream& ostream, ModuleManager& moduleManager) const override;
		Object* readObject(std::istream& istream, Core& core, Object* parent = nullptr) const override;
	};

	class JSONResource : public Resource
	{
        RTTI_ENABLE_DERIVED_FROM(Resource)
	public:
		JSONResource(const std::string& path, const std::string& contents) : mContents(contents), mPath(path) {}

        Object* createInstance(Core& core, Object& parent)
		{
            return JSONSerializer().fromString(mContents, core, &parent);
        }

		virtual const std::string getDisplayName() const override { return mPath; }

    private:
		const std::string mContents;
		std::string mPath;
	};



	class JSONFileLoader : public ResourceLoader
	{
        RTTI_ENABLE_DERIVED_FROM(ResourceLoader)
	public:
		JSONFileLoader() : ResourceLoader() { addFileExtension("napj"); }

		std::unique_ptr<Resource> loadResource(const std::string& resourcePath) const override;

	private:
		JSONSerializer mSerializer;
	};
}

RTTI_DECLARE(nap::JSONSerializer)
RTTI_DECLARE_BASE(nap::JSONResource)
RTTI_DECLARE(nap::JSONFileLoader)
