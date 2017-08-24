#pragma once

#include "appcontext.h"
#include "filetype.h"
#include <QPointF>
#include <QString>
#include <QtCore/QBuffer>
#include <QtCore/QFileInfo>
#include <QtCore/QXmlStreamWriter>
#include <nap/operator.h>
#include <nap/serializer.h>

// Based on a given filename, return that filename that is guaranteed to have
// the provided suffix
QString ensureExtension(const QString& filename, const QString& extension);

// Fetch an object's 2d editor position
QPointF getObjectEditorPosition(nap::AttributeObject& object);

// Set an object's position in the editor
void setObjectEditorPosition(nap::AttributeObject& object, const QPointF& pos);

bool canConnect(const nap::Plug& outPlug, const nap::Plug& inPlug);

// Return given value, where: min <= value <= max
template <typename T>
inline T CLAMP(T value, T min, T max)
{
	if (value < min) return min;
	if (value > max) return max;
	return value;
}


// Return a filtered list where none of the elements is a child of another.
QList<nap::Object*> keepRoots(const QList<nap::Object*>& objects);

// See if child is owned by parent, directly or indirectly
bool isChildOf(const nap::Object& child, const nap::Object& parent);

// Set the provided attribute value from the provided string
void attributeFromString(nap::AttributeBase& at, const QString& value);

// Return the provided attribute's value as string
QString attributeToString(nap::AttributeBase& at);

void calculateWirePath(QPointF mSrcPos, QPointF mDstPos, QPainterPath& p);

void selectionToClipboard();

std::string stripNamespace(const std::string& name);

// Get the representation color for a datatype
QColor dataTypeColor(const RTTI::TypeInfo& type);

QString fileExtension(const QString& filename);

FileType* fileTypeFromFilename(const QString& filename);

QString fileTypesFilter();

FileType* defaultFileType();