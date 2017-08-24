
#include <QIcon>
#include <QMap>
#include <nap/entity.h>
#include <nap/patchcomponent.h>
#include <rtti/rtti.h>

class IconStore
{
public:
	IconStore()
	{
		addIcon(RTTI_OF(nap::Component), "diamond-orange");
		addIcon(RTTI_OF(nap::Entity), "cube-blue");
		addIcon(RTTI_OF(nap::Module), "package");
		addIcon(RTTI_OF(nap::AttributeBase), "bullet_green");
        addIcon(RTTI_OF(nap::Patch), "node-select-all");
	}

	void addIcon(const RTTI::TypeInfo& type, const QString& name) { mIconsByType[type] = name; }

	const QIcon* get(const QString& name)
	{
		if (!mIconMap.contains(name)) {
			QIcon* icon = new QIcon("resources/" + name + ".png");
			if (icon->isNull()) {
				nap::Logger::fatal("Failed to load icon '%s'", name.toStdString().c_str());
				return nullptr;
			}
			mIconMap.insert(name, icon);
		}
		return mIconMap[name];
	}

    const QIcon* iconFor(const RTTI::TypeInfo& type)
    {
        if (!mIconsByType.contains(type)) {
            nap::Logger::warn("No icon for type '%s'", type.getName().c_str());
            return nullptr;
        }
        return get(mIconsByType[type]);
    }

	const QIcon* iconFor(const nap::Object& obj)
	{
		for (const auto& type : mIconsByType.keys()) {
			if (obj.getTypeInfo().isKindOf(type)) return get(mIconsByType[type]);
		}
        nap::Logger::warn("No icon for type '%s'", obj.getTypeInfo().getName().c_str());
		return nullptr;
	}

private:
	QMap<RTTI::TypeInfo, QString> mIconsByType;
	QMap<QString, const QIcon*> mIconMap;
};