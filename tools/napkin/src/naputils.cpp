#include <QtGui/QtGui>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDir>
#include <QUrl>
#include <QLabel>
#include <QDialogButtonBox>

#include <component.h>
#include <nap/logger.h>
#include <entity.h>
#include <standarditemsproperty.h>
#include <standarditemsobject.h>
#include <panels/finderpanel.h>

#include <napqt/filterpopup.h>

#include "naputils.h"
#include "napkinglobals.h"
#include "appcontext.h"

using namespace nap::rtti;
using namespace nap::utility;
using namespace napkin;

napkin::RTTITypeItem::RTTITypeItem(const nap::rtti::TypeInfo& type) : type(type)
{
	setText(type.get_name().data());
	setEditable(false);
	//    setForeground(getSoftForeground());
	//    setBackground(getSoftBackground());
	refresh();
}

napkin::FlatObjectModel::FlatObjectModel(const std::vector<Object*> objects)
{
	for (auto object : objects)
	{
		auto item = new ObjectItem(object, false);
		item->setEditable(false);
		appendRow(item);
	}
}


void napkin::RTTITypeItem::refresh()
{
	for (const nap::rtti::TypeInfo& derived : type.get_derived_classes())
	{
		appendRow(new RTTITypeItem(derived));
	}
}

nap::rtti::ObjectList napkin::topLevelObjects(const ObjectList& objects)
{
	// Pass 1: determine the set of all potential root objects
	std::vector<ObjectLink> all_object_links;
	ObjectSet allObjects;
	ObjectList objects_to_visit = objects;
	for (int index = 0; index < objects_to_visit.size(); ++index)
	{
		Object* object = objects_to_visit[index];
		allObjects.insert(object);

		// Find links for all objects
		std::vector<ObjectLink> links;
		findObjectLinks(*object, links);

		// Visit each links; the target of each link is a potential root object
		all_object_links.reserve(all_object_links.size() + links.size());
		objects_to_visit.reserve(objects_to_visit.size() + links.size());
		for (ObjectLink& link : links)
		{
			if (link.mTarget != nullptr && allObjects.find(link.mTarget) == allObjects.end())
				objects_to_visit.push_back(link.mTarget);

			all_object_links.push_back(link);
		}
	}

	// Pass 2: now that we know all potential root objects, build the list of actual root object
	// An object is a root object if it is not pointed to by an embedded pointer, or if it's pointed to by an embedded
	// pointer but the writer does not support embedded pointers
    ObjectList topLevelObjects; // RVO will take care of this
	topLevelObjects.reserve(allObjects.size());
	for (Object* object : allObjects)
	{
		bool is_embedded_object = false;

		// Scan through all links to figure out if any embedded pointer is pointing to this object.
		for (auto& link : all_object_links)
		{
			if (link.mTarget != object)
				continue;

			ResolvedPath resolved_path;
			link.mSourcePath.resolve(link.mSource, resolved_path);

			auto property = resolved_path.getProperty();
			if (hasFlag(property, EPropertyMetaData::Embedded))
			{
				is_embedded_object = true;
				break;
			}
		}

		// Only non-embedded objects can be roots
		if (!is_embedded_object)
			topLevelObjects.push_back(object);
	}

    return topLevelObjects;
}

nap::rtti::Object* napkin::showObjectSelector(QWidget* parent, const std::vector<nap::rtti::Object*>& objects)
{
	QStringList ids;
	for (auto& obj : objects)
		ids << QString::fromStdString(obj->mID);

	auto selectedID = nap::qt::FilterPopup::show(parent, ids).toStdString();
	if (selectedID.empty())
		return nullptr;

	auto it = std::find_if(objects.begin(), objects.end(), [selectedID](auto& a) { return a->mID == selectedID; });
	if (it == objects.end())
		return nullptr;

	return *it;
}

nap::rtti::TypeInfo napkin::showTypeSelector(QWidget* parent, const TypePredicate& predicate)
{
	QStringList names;
	for (const auto& t : getTypes(predicate))
		names << QString::fromStdString(std::string(t.get_name().data()));

	auto selectedName = nap::qt::FilterPopup::show(parent, names).toStdString();
	return nap::rtti::TypeInfo::get_by_name(selectedName.c_str());
}


std::vector<rttr::type> napkin::getComponentTypes()
{
	std::vector<rttr::type> ret;
	nap::rtti::TypeInfo rootType = RTTI_OF(nap::Component);
	for (const nap::rtti::TypeInfo& derived : rootType.get_derived_classes())
	{
		if (derived.can_create_instance())
			ret.emplace_back(derived);
	}
	return ret;
}

std::vector<rttr::type> napkin::getTypes(TypePredicate predicate)
{
	nap::rtti::Factory& factory = AppContext::get().getCore().getResourceManager()->getFactory();
	std::vector<rttr::type> ret;
	std::vector<rttr::type> derived_classes;

	auto rootType = RTTI_OF(nap::rtti::Object);
	nap::rtti::getDerivedTypesRecursive(rootType, derived_classes);
	for (const rttr::type& derived : derived_classes)
	{
		if (!factory.canCreate(derived))
			continue;

		if (predicate != nullptr && !predicate(derived))
			continue;

		ret.emplace_back(derived);
	}
	return ret;
}

nap::rtti::ResolvedPath napkin::resolve(const nap::rtti::Object& obj, nap::rtti::Path path)
{
	nap::rtti::ResolvedPath resolvedPath;
	path.resolve(&obj, resolvedPath);
	assert(resolvedPath.isValid());
	return resolvedPath;
}


QString napkin::getAbsoluteResourcePath(const QString& relPath, const QString& reference)
{
	auto ref = getResourceReferencePath(reference);
	return QFileInfo(QString("%1/%2").arg(ref, relPath)).canonicalFilePath();
}

QString napkin::getRelativeResourcePath(const QString& absPath, const QString& reference)
{
	auto ref = getResourceReferencePath(reference);
	return QDir(ref).relativeFilePath(absPath);
}

QString napkin::getResourceReferencePath(const QString& reference)
{
	QString ref = reference;
	if (reference.isEmpty())
	{
		auto reffile = AppContext::get().getDocument()->getCurrentFilename();
		ref = QFileInfo(reffile).path();
	}

	QFileInfo refinfo(ref);
	if (refinfo.isFile())
		ref = refinfo.path();

	return ref;
}

std::string napkin::toLocalURI(const std::string& filename)
{

	return QUrl::fromLocalFile(QString::fromStdString(filename)).toString().toStdString();
}

std::string napkin::fromLocalURI(const std::string& fileuri)
{
	return QUrl(QString::fromStdString(fileuri)).toLocalFile().toStdString();
}


std::string napkin::toURI(const nap::rtti::Object& object)
{
	return NAP_URI_PREFIX + "://" + object.mID;
}

std::string napkin::toURI(const napkin::PropertyPath& path)
{
	return NAP_URI_PREFIX + "://" + path.toString();
}

bool napkin::showPropertyListConfirmDialog(QWidget* parent, QList<PropertyPath> props, const QString& title,
										   QString message)
{
	QDialog dialog(parent);
	dialog.setWindowTitle(title);

	QVBoxLayout layout;
	dialog.setLayout(&layout);

	QLabel label;
	label.setText(message);
	layout.addWidget(&label);

	FinderPanel finder;
	nap::qt::FilterTreeView* tree = &finder.getTreeView();
	tree->getTreeView().setHeaderHidden(true);
	tree->getTreeView().setRootIsDecorated(false);

	// On item double-click, close the dialog and reveal the property
	finder.connect(&tree->getTreeView(), &QAbstractItemView::doubleClicked,
				   [tree, &dialog](const QModelIndex& idx)
	{
		const auto sourceIndex = tree->getFilterModel().mapToSource(idx);
		auto item = tree->getModel()->itemFromIndex(sourceIndex);
		auto propitem = dynamic_cast<PropertyDisplayItem*>(item);
		assert(propitem);
		napkin::AppContext::get().propertySelectionChanged(propitem->getPath());
		dialog.close();
	});

	finder.setPropertyList(props);
	layout.addWidget(&finder);

	QDialogButtonBox buttonBox(QDialogButtonBox::Yes | QDialogButtonBox::No);
	layout.addWidget(&buttonBox);

	bool yesclicked = false;

	QPushButton* btYes = buttonBox.button(QDialogButtonBox::Yes);
	dialog.connect(btYes, &QPushButton::clicked, [&dialog, &yesclicked]() {
		yesclicked = true;
		dialog.close();
	});

	QPushButton* btNo = buttonBox.button(QDialogButtonBox::No);
	dialog.connect(btNo, &QPushButton::clicked, [&dialog, &yesclicked]() {
		dialog.close();
	});

	dialog.exec();

	return yesclicked;
}

std::string napkin::friendlyTypeName(rttr::type type)
{
	// Strip off namespace prefixes when creating new objects
	std::string base_name = type.get_name().data();
	size_t last_colon = base_name.find_last_of(':');
	if (last_colon != std::string::npos)
		base_name = base_name.substr(last_colon + 1);
	return base_name;
}

bool napkin::isComponentInstancePathEqual(const nap::RootEntity& rootEntity, const nap::Component& comp,
								  const std::string& a, const std::string& b)
{
	{
		auto partsA = nap::utility::splitString(a, '/');
		auto partsB = nap::utility::splitString(b, '/');

		assert(partsA[0] == ".");
		assert(partsB[0] == ".");

		std::string nameA;
		std::string nameB;
		int indexA;
		int indexB;

		for (size_t i = 1, len = partsA.size(); i < len; i++)
		{
			const auto& partA = partsA[i];
			const auto& partB = partsB[i];

			// continue finding child entities
			bool hasIndexA = nameAndIndex(partA, nameA, indexA);
			bool hasIndexB = nameAndIndex(partB, nameB, indexB);

			// consider no index to be index 0
			if (!hasIndexA)
				indexA = 0;
			if (!hasIndexB)
				indexB = 0;

			if (nameA != nameB)
				return false;

			if (indexA != indexB)
				return false;

		}
		return true;
	}
}

bool napkin::nameAndIndex(const std::string& nameIndex, std::string& name, int& index)
{
	std::size_t found = nameIndex.find_last_of(':');
	if (found != std::string::npos)
	{
		auto n = nameIndex.substr(0, found);
		auto i = nameIndex.substr(found + 1);
		if (isNumber(i))
		{
			name.assign(n);
			index = std::stoi(i);
			return true;
		}
	}
	name.assign(nameIndex);
	return false;
}

bool napkin::isNumber(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(), s.end(),
									  [](char c)
									  {
										  return !std::isdigit(c);
									  }) == s.end();
}

nap::Entity* napkin::findChild(nap::Entity& parent, const std::string& name, int index)
{
	if (index < 0)
	{
		// Just find by name
		for (auto e : parent.mChildren)
			if (e->mID == name)
				return e.get();
	}
	else
	{
		int nameFound = 0;
		for (auto e : parent.mChildren)
		{
			if (e->mID != name)
			continue;

			if (nameFound == index)
				return e.get();

			++nameFound;
		}
	}
	return nullptr;
}
