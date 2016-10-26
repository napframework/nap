// Local Includes
#include "napkin_utilities.h"

// External Includes
#include <QtCore/qtextstream.h>
#include <nap/coreattributes.h>


QString ensureExtension(const QString& filename, const QString& extension)
{
	QFileInfo info(filename);

	// Already okay
	if (info.suffix() == extension) return filename;

	// No extension
	if (info.suffix().isNull() || info.suffix().isEmpty()) return QString("%1.%2").arg(filename, extension);

	// Wrong extension
	return QString("%s/%1.%2").arg(info.path(), info.baseName(), extension);
}

static const std::string PATCH_OP_XPOS = "__patch_op_xpos";
static const std::string PATCH_OP_YPOS = "__patch_op_ypos";

QPointF getObjectEditorPosition(nap::AttributeObject& object)
{
	nap::Attribute<float>* attrX = object.getOrCreateAttribute<float>(PATCH_OP_XPOS);
	nap::Attribute<float>* attrY = object.getOrCreateAttribute<float>(PATCH_OP_YPOS);

	return QPointF(attrX->getValue(), attrY->getValue());
}

void setObjectEditorPosition(nap::AttributeObject& object, const QPointF& pos)
{
	nap::Attribute<float>* attrX = object.getOrCreateAttribute<float>(PATCH_OP_XPOS);
	nap::Attribute<float>* attrY = object.getOrCreateAttribute<float>(PATCH_OP_YPOS);

	attrX->setValue(pos.x());
	attrY->setValue(pos.y());
}


void attributeFromString(nap::AttributeBase& at, const QString& value)
{
	const nap::TypeConverterBase* tc =
		AppContext::get().core().getModuleManager().getTypeConverter(RTTI_OF(std::string), at.getValueType());
	if (tc) {
		nap::Attribute<std::string> stringAttr;
		stringAttr.setValue(value.toStdString());
		tc->convert(&stringAttr, &at);
		return;
	}
	// TODO: Go for type converters
	at.fromString(value.toStdString());
}

QString attributeToString(nap::AttributeBase& at)
{
	const nap::TypeConverterBase* tc =
		AppContext::get().core().getModuleManager().getTypeConverter(at.getValueType(), RTTI_OF(std::string));

	if (tc) {
		nap::Attribute<std::string> stringAttr;
		tc->convert(&at, &stringAttr);
		return QString::fromStdString(stringAttr.getValue());
	}
	// TODO: Go for typeconverters
	std::string string;
	at.toString(string);
	return QString::fromStdString(string);
}

QList<nap::Object*> keepRoots(const QList<nap::Object*>& objects)
{
	QList<nap::Object*> roots;
	for (int i = 0; i < objects.size(); i++) {
		auto child = objects[i];

		bool hasParent = false;
		for (int j = i + 1; j < objects.size(); j++) {
			auto parent = objects[j];
			if (isChildOf(*child, *parent)) {
				hasParent = true;
				break;
			}
		}
		if (!hasParent) roots << child;
	}
	return roots;
}


bool isChildOf(const nap::Object& child, const nap::Object& parent)
{
	const nap::Object* current = &child;
	while (current->getParentObject()) {
		if (current->getParentObject() == &parent) return true;
		current = current->getParentObject();
	}
	return false;
}


void calculateWirePath(QPointF mSrcPos, QPointF mDstPos, QPainterPath& p)
{
	p.moveTo(mSrcPos);

	if (mDstPos.x() > mSrcPos.x()) {
		qreal hx = mSrcPos.x() + (mDstPos.x() - mSrcPos.x()) / 2.0;
		QPointF c1 = QPointF(hx, mSrcPos.y());
		QPointF c2 = QPointF(hx, mDstPos.y());
		p.cubicTo(c1, c2, mDstPos);
	} else {
		// Wire is moving back, folding over itself
		qreal maxDist = 50;

		qreal dist = CLAMP(mSrcPos.x() - mDstPos.x(), -maxDist, maxDist);

		QPointF ca1(mSrcPos.x() + dist, mSrcPos.y());
		QPointF c((mDstPos.x() + mSrcPos.x()) / 2, (mDstPos.y() + mSrcPos.y()) / 2);
		QPointF ca2(mSrcPos.x() + dist, (mSrcPos.y() + c.y()) / 2);

		QPointF cb1(mDstPos.x() - dist, (mDstPos.y() + c.y()) / 2);
		QPointF cb2(mDstPos.x() - dist, mDstPos.y());

		p.cubicTo(ca1, ca2, c);
		p.cubicTo(cb1, cb2, mDstPos);
	}
}


bool canConnect(const nap::Plug& outPlug, const nap::Plug& inPlug)
{
	// Cannot connect outlets to outlets or inlets to inlets
	if (inPlug.getTypeInfo().isKindOf<nap::InputPlugBase>() && outPlug.getTypeInfo().isKindOf<nap::InputPlugBase>())
		return false;
	if (inPlug.getTypeInfo().isKindOf<nap::OutputPlugBase>() && outPlug.getTypeInfo().isKindOf<nap::OutputPlugBase>())
		return false;

	// Cannot connect to the same operator
	if (outPlug.getParent() == inPlug.getParent()) return false;

	// Cannot connect to incompatible dataTypes
	if (outPlug.getDataType() != inPlug.getDataType()) return false;

	// Ok!
	return true;
}

void selectionToClipboard()
{
	const auto& selection = AppContext::get().selection().toStdList();

	std::stringstream ss;
	nap::XMLSerializer ser(ss, AppContext::get().core());
	for (auto ob : selection)
		ser.writeObject(*ob);
	std::string buffer = ss.str();

	QApplication::clipboard()->setText(QString::fromStdString(buffer));
}

QColor dataTypeColor(const RTTI::TypeInfo& type)
{
	float h = type.getId();
	static float goldenRatioConjugate = 0.618033988749895f;
	float hh = (float)fmod(goldenRatioConjugate * h, 1);
	QColor col;
	col.setHsvF(hh, 0.8f, 0.55f);
	return col;
}
std::string stripNamespace(const std::string& name) { return name.substr(name.find_last_of(':') + 1); }
QString fileExtension(const QString& filename) { return QFileInfo(filename).suffix(); }
FileType* fileTypeFromFilename(const QString& filename)
{
	QString ext = fileExtension(filename);

	if (ext.isNull() || ext.isEmpty()) return nullptr;

	for (const auto& type : getFileTypes()) {
		if (QString::fromStdString(type.get()->extension()) == ext) return type.get();
	}

	nap::Logger::fatal("Failed to detect filetype from filename '%s'", filename.toStdString().c_str());

	return nullptr;
}

QString fileTypesFilter()
{
	QString filter;
	QTextStream s(&filter);
	s << "Nap File (";

	const auto& types = getFileTypes();
	for (int i = 0; i < types.size(); i++) {
		if (i > 0) s << " ";
		s << "*.";
		s << QString::fromStdString(types[i].get()->extension());
	}
	s << ")";

	return filter;
}
FileType* defaultFileType() { return getFileTypes()[0].get(); }
