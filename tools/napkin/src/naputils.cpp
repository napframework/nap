/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "naputils.h"
#include "napkinglobals.h"
#include "appcontext.h"

#include <QtGui/QtGui>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDir>
#include <QUrl>
#include <QLabel>
#include <QDialogButtonBox>

#include <component.h>
#include <entity.h>
#include <standarditemsobject.h>
#include <panels/finderpanel.h>
#include <cctype>
#include <napqt/filterpopup.h>
#include <renderservice.h>

using namespace nap::rtti;
using namespace nap::utility;
using namespace napkin;

RTTI_DEFINE_BASE(napkin::RTTITypeItem)

std::vector<rttr::type> napkin::getImmediateDerivedTypes(const rttr::type& type)
{
	// Cycle over all derived types. This includes all derived types,
	// including classes that are more than 1 level up. We're only 
	// interested in immediate derived types, so check if the first
	// available base type is the type we're interested in and add
	std::vector<rttr::type> derivedTypes;
	for (const nap::rtti::TypeInfo& derived : type.get_derived_classes())
	{
		// Check if the first available base class (immediate base level)
		// is directly derived from the type we're searching for.
		assert(!derived.get_base_classes().empty());
		rttr::type base = *derived.get_base_classes().begin();
		if (base == type)
			derivedTypes.emplace_back(derived);
	}
	return derivedTypes;
}


void napkin::dumpTypes(rttr::type type, const std::string& indent)
{
	const void * address = static_cast<const void*>(&type);
	std::stringstream ss;
	ss << address;
	std::string name = ss.str();

	nap::Logger::info("type: " + indent + type.get_name().data() + " (" + ss.str() + ")");
	for (const auto& derived : getImmediateDerivedTypes(type))
		dumpTypes(derived, indent + "    ");
}


napkin::RTTITypeItem::RTTITypeItem(const nap::rtti::TypeInfo& type) : mType(type)
{
	QString type_name(type.get_name().data());
	auto parts = type_name.split(','); assert(parts.size() > 0);
	parts.first().remove("class ");
	setText(parts.first());
	setEditable(false);
}


QVariant napkin::RTTITypeItem::data(int role) const
{
	if (role == Qt::ForegroundRole)
	{
		return QVariant::fromValue<QColor>(AppContext::get().getThemeManager().getColor(theme::color::dimmedItem));
	}
	return QStandardItem::data(role);
}


napkin::FlatObjectModel::FlatObjectModel(const std::vector<Object*> objects)
{
	for (auto object : objects)
	{
		auto item = new ObjectItem(*object);
		item->setEditable(false);
		appendRow(item);
	}
}


nap::rtti::ObjectSet napkin::topLevelObjects()
{
	// Bail if no document is loaded
	ObjectSet top_level_objects; // RVO will take care of this
	napkin::Document* doc = AppContext::get().getDocument();
	if (doc == nullptr)
		return top_level_objects;

	// Create set that contains all the objects
	const auto& all_objects = doc->getObjects();
	top_level_objects.reserve(all_objects.size());
	for (const auto& it : all_objects)
		top_level_objects.emplace(it.second.get());

	// Filter by removing all embedded objects
	for (const auto& it : all_objects)
	{
		// Get all links
		std::vector<ObjectLink> links;
		findObjectLinks(*it.second, links);
		for (const auto& link : links)
		{
			// Check if the link points to an embedded resource
			ResolvedPath resolved_path;
			link.mSourcePath.resolve(link.mSource, resolved_path);

			// Remove as top level object
			if (hasFlag(resolved_path.getProperty(), EPropertyMetaData::Embedded))
			{
				top_level_objects.erase(link.mTarget);
			}
		}
	}
    return top_level_objects;
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


nap::rtti::TypeInfo napkin::showMaterialSelector(QWidget* parent, const PropertyPath& prop, std::string& outName)
{
	auto* material = rtti_cast<nap::Material>(prop.getObject());
	assert(material != nullptr);
	if (material->mShader == nullptr)
		return nap::rtti::TypeInfo::empty();

	QStringList names;
	const auto& ubo_decs = material->mShader->getUBODeclarations();
	for (const auto& dec : ubo_decs)
	{
		names << QString::fromStdString(dec.mName);
	}

	auto selectedID = nap::qt::FilterPopup::show(parent, names).toStdString();
	return nap::rtti::TypeInfo::empty();
}


nap::rtti::TypeInfo napkin::showTypeSelector(QWidget* parent, const TypePredicate& predicate)
{
	QStringList names;
	for (const auto& t : getTypes(predicate))
		names << QString::fromStdString(std::string(t.get_name().data()));

	auto selectedName = nap::qt::FilterPopup::show(parent, names).toStdString();
	return selectedName.empty() ? nap::rtti::TypeInfo::empty() : nap::rtti::TypeInfo::get_by_name(selectedName.c_str());
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
	std::vector<rttr::type> ret;
	nap::Core& core = AppContext::get().getCore();
	if (!core.isInitialized())
		return ret;

	nap::rtti::Factory& factory = core.getResourceManager()->getFactory();
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
		auto reffile = AppContext::get().getDocument()->getFilename();
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
	return std::string(NAP_URI_PREFIX) + "://" + object.mID;
}

std::string napkin::toURI(const napkin::PropertyPath& path)
{
	return std::string(NAP_URI_PREFIX) + "://" + path.toString();
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
		const auto sourceIndex = tree->getProxyModel().mapToSource(idx);
		auto item = tree->getModel()->itemFromIndex(sourceIndex);
		auto propitem = qitem_cast<PropertyDisplayItem*>(item);
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


bool napkin::loadShader(nap::BaseShader& shader, nap::Core& core, nap::utility::ErrorState& error)
{
	// Only load shader when a render service is available
	if (!AppContext::get().canRender())
		return false;

	// Change working directory for compilation
	auto cwd = nap::utility::getCWD();
	assert(core.isInitialized());
	nap::utility::changeDir(core.getProjectInfo()->getDataDirectory());

	// Clear and load
	shader.clear();
	bool success = shader.init(error);
	nap::utility::changeDir(cwd);
	return success;
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


bool napkin::isComponentInstancePathEqual(const std::string& a, const std::string& b)
{
	auto parts_a = nap::utility::splitString(a, '/'); assert(parts_a[0] == ".");
	auto parts_b = nap::utility::splitString(b, '/'); assert(parts_a[0] == ".");

	std::string name_a; int index_a;
	std::string name_b; int index_b;
	for (size_t i = 1, len = parts_a.size(); i < len; i++)
	{
		// continue finding child entities, no index = 0
		if (!nameAndIndex(parts_a[i], name_a, index_a))
			index_a = 0;

		if (!nameAndIndex(parts_b[i], name_b, index_b))
			index_b = 0;

		if (name_a != name_b)
			return false;

		if (index_a != index_b)
			return false;
	}
	return true;
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
										  return !std::isdigit(static_cast<unsigned char>(c));
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
