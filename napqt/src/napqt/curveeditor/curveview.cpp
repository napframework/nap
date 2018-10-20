#include <napqt/timeline/eventitem.h>
#include "curveview.h"
#include "curvemath.h"
#include <QtDebug>
#include <QMap>
#include <cassert>
#include <QVector>

using namespace napqt;

#define DEFAULT_SCENE_EXTENT 1000


QList<int> reverseSort(const QList<int>& ints)
{
	auto sortedIndices = ints;
	qSort(sortedIndices.begin(), sortedIndices.end(), [](const QVariant& a, const QVariant& b)
	{
		bool ok;
		return b.toInt(&ok) < a.toInt(&ok);
	});
	return sortedIndices;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CurveItem
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HandleItem::HandleItem(CurveSegmentItem& parent) : QObject(), QGraphicsItem(&parent)
{
	setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	setFlag(QGraphicsItem::ItemIsMovable, true);
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);

	mHitRect.setCoords(-mHitExtent, -mHitExtent, mHitExtent * 2, mHitExtent * 2);
	updateRect();
}

bool HandleItem::contains(const QPointF& point) const
{
	return mHitRect.contains(point);
}

QRectF HandleItem::boundingRect() const
{
	return mHitRect;
}

QVariant HandleItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
	QVariant res = QGraphicsItem::itemChange(change, value);
	if (change == QGraphicsItem::ItemPositionChange)
	{
		if (mEmitItemChanges)
			moved(this);
	} else if (change == QGraphicsItem::ItemSelectedHasChanged)
	{
		updateRect();
		if (mEmitItemChanges)
			selected(this);
	}
	return res;
}

void HandleItem::updateRect()
{
	mPath = QPainterPath();
	auto ext = isSelected() ? mExtentSelectd : mExtent;
	mRect.setCoords(-ext, -ext, ext * 2, ext * 2);
	mPath.addRect(mRect);
}

void HandleItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setPen(isSelected() ? mPenSelected : mPen);
	painter->setBrush(isSelected() ? mBrushSelected : mBrush);
	painter->drawPath(mPath);
}

CurveSegmentItem& HandleItem::curveSegmentItem()
{
	return *dynamic_cast<CurveSegmentItem*>(parentItem());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PointHandleItem
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PointHandleItem::PointHandleItem(CurveSegmentItem& parent) : HandleItem(parent)
{
	mPen = QPen(Qt::NoPen);
	mPenSelected = QPen(QColor("#000"), 0, Qt::SolidLine);
	mBrush = QBrush("#840");
	mBrushSelected = QBrush("#FFF");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TangentHandleItem
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TangentHandleItem::TangentHandleItem(CurveSegmentItem& parent) : HandleItem(parent)
{
	mPen = QPen(Qt::NoPen);
	mBrush = QBrush("#080");
	mBrushSelected = QBrush("#0F0");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TangentLineItem
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LineItem::LineItem(QGraphicsItem& parent) : QGraphicsPathItem(&parent)
{
	setZValue(-100);
	setColor("#F00");
}

void LineItem::setColor(const QColor& color)
{
	setPen(QPen(QBrush(color), 0));
}

void LineItem::setFromTo(const QPointF& a, const QPointF& b)
{
	QPainterPath p;
	p.moveTo(a);
	p.lineTo(b);
	setPath(p);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TangentLineItem
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CurveSegmentItem::CurveSegmentItem(CurveItem& curveItem)
		: QObject(), QGraphicsItem(&curveItem),
		  mPointHandle(*this),
		  mInTanHandle(*this),
		  mOutTanHandle(*this),
		  mInTanLine(*this),
		  mOutTanLine(*this)
{
	setZValue(-200);
	setFlag(QGraphicsItem::ItemIsSelectable, true);

	mPen = QPen(Qt::blue, 0);
	mPenDebug = QPen(Qt::gray, 0);
	mPenSelected = QPen(Qt::red, 0);

	connect(&mPointHandle, &HandleItem::moved, this, &CurveSegmentItem::onHandleMoved);
	connect(&mInTanHandle, &HandleItem::moved, this, &CurveSegmentItem::onHandleMoved);
	connect(&mOutTanHandle, &HandleItem::moved, this, &CurveSegmentItem::onHandleMoved);
	connect(&mPointHandle, &HandleItem::selected, this, &CurveSegmentItem::updateHandleVisibility);
}

CurveItem& CurveSegmentItem::curveItem() const
{
	auto item = dynamic_cast<CurveItem*>(parentItem());
	assert(item);
	return *item;
}

QRectF CurveSegmentItem::boundingRect() const
{
	return mPath.boundingRect().united(mDebugPath.boundingRect()).united(childrenBoundingRect());
}

void CurveSegmentItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setPen(isSelected() ? mPenSelected : mPen);
	painter->drawPath(mPath);

	if (mDrawDebug)
	{
		painter->setPen(mPenDebug);
		painter->drawPath(mDebugPath);
	}
}

int CurveSegmentItem::index()
{
	return curveItem().segmentIndex(*this);
}

int CurveSegmentItem::orderedIndex()
{
	return curveItem().sortedIndex(*this);
}

QPainterPath CurveSegmentItem::shape() const
{
	return mPath;
}

void CurveSegmentItem::updateGeometry()
{
	auto& curve = curveItem().curve();
	int segCount = curve.pointCount();
	int idx = index();
	int orderedIdx = orderedIndex();

	bool isLast = orderedIdx == segCount - 1;

	auto pos = curve.data(idx, datarole::POS).toPointF();
	auto inTan = curve.data(idx, datarole::IN_TAN).toPointF();
	auto outTan = curve.data(idx, datarole::OUT_TAN).toPointF();
	auto interp = curve.data(idx, datarole::INTERP).value<AbstractCurve::InterpType>();
	auto c1 = pos + outTan;
	auto inTanPos = pos + inTan;
	auto outTanPos = pos + outTan;

	mPointHandle.setPos(pos);
	mInTanHandle.setPos(inTanPos);
	mOutTanHandle.setPos(outTanPos);

	mInTanLine.setFromTo(pos, inTanPos);
	mOutTanLine.setFromTo(pos, outTanPos);

	mPath = QPainterPath();
	mDebugPath = QPainterPath();

	if (!isLast)
	{
		// Handle anything that needs the next segment/point
		auto nextSeg = curveItem().nextSegment(*this);
		auto nextIdx = curveItem().segmentIndex(*nextSeg);
		auto nextInTan = curve.data(nextIdx, datarole::IN_TAN).toPointF();
		auto nextPos = curve.data(nextIdx, datarole::POS).toPointF();
		auto c2 = nextPos + nextInTan;

		// TODO: Calulate debug path

		mPath.moveTo(pos);
		mPath.cubicTo(c1, c2, nextPos);
	}

	updateHandleVisibility();
	QGraphicsItem::update();
}

void CurveSegmentItem::updateHandleVisibility()
{
	auto& curve = curveItem().curve();
	int segCount = curve.pointCount();
	int idx = index();
	int orderedIdx = orderedIndex();

	bool isLast = orderedIdx == segCount - 1;
	bool isFirst = orderedIdx == 0;

	bool s = mPointHandle.isSelected();

	mInTanLine.setVisible(!isFirst && s);
	mInTanHandle.setVisible(!isFirst && s);
	mOutTanLine.setVisible(!isLast && s);
	mOutTanHandle.setVisible(!isLast && s);
}

void CurveSegmentItem::onHandleMoved(HandleItem* handle)
{
	handleMoved(handle);
}

void CurveSegmentItem::onHandleSelected(HandleItem* handle)
{
	updateHandleVisibility();
}

void CurveSegmentItem::setPointsEmitItemChanges(bool b)
{
	mPointHandle.setEmitItemChanged(b);
	mInTanHandle.setEmitItemChanged(b);
	mOutTanHandle.setEmitItemChanged(b);
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CurveItem
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


CurveItem::CurveItem(QGraphicsItem* parent, AbstractCurve& curve)
		: QObject(), QGraphicsPathItem(parent),
		  mCurve(curve),
		  mInfinityLineNeg(*this),
		  mInfinityLinePos(*this)
{
	connect(&curve, &AbstractCurve::pointsChanged, this, &CurveItem::onPointsChanged);
	connect(&curve, &AbstractCurve::pointsAdded, this, &CurveItem::onPointsAdded);
	connect(&curve, &AbstractCurve::pointsRemoved, this, &CurveItem::onPointsRemoved);

	QList < int > indices;
	for (int i = 0, len = curve.pointCount(); i < len; i++)
		indices << i;
	onPointsAdded(indices);
}

void CurveItem::onPointsChanged(QList<int> indices)
{
	setPointOrderDirty();
	for (int i = 0, len = indices.size(); i < len; i++)
		updateSegmentFromPoint(i);
}

void CurveItem::updateSegmentFromPoint(int i)
{
	CurveSegmentItem* seg = mSegments[i];
	seg->updateGeometry();
	if (!isLastPoint(segmentIndex(*seg)))
		nextSegment(*seg)->updateGeometry();
}

bool CurveItem::isFirstPoint(int i)
{
	sortPoints();
	return sortedIndex(i) == 0;
}

bool CurveItem::isLastPoint(int i)
{
	sortPoints();
	return sortedIndex(i) == mSortedToUnsorted.size() - 1;
}

CurveSegmentItem* CurveItem::nextSegment(const CurveSegmentItem& seg)
{
	const auto& sortedIndices = sortPoints();
	int idx = segmentIndex(seg);
	int outidx = mUnsortedToSorted[idx + 1];
	return mSegments[outidx];
}

void CurveItem::onPointsAdded(const QList<int> indices)
{
	QList < CurveSegmentItem * > segments;
	int indexCount = indices.size();
	for (int index : indices)
	{
		auto seg = new CurveSegmentItem(*this);
		seg->setParentItem(this);
		if (index == curve().pointCount())
			mSegments.append(seg);
		else
			mSegments.insert(index, seg);
		segments << seg;
	}

	// Push changes from model too
	QList<int> idx;
	for (auto seg : segments)
		idx << segmentIndex(*seg);

	setPointOrderDirty();
	onPointsChanged(idx);
}

void CurveItem::onPointsRemoved(const QList<int> indices)
{
	for (int index : reverseSort(indices))
	{
		auto seg = mSegments.takeAt(index);
		delete seg;
	}
	setPointOrderDirty();
}

int CurveItem::segmentIndex(const CurveSegmentItem& item) const
{
	for (int i = 0, len = mSegments.size(); i < len; i++)
		if (mSegments.at(i) == &item)
			return i;
	return -1;
}

int CurveItem::sortedIndex(const CurveSegmentItem& seg)
{
	sortPoints();
	return mSortedToUnsorted[segmentIndex(seg)];
}

int CurveItem::sortedIndex(int unsortedIndex)
{
	sortPoints();
	return mSortedToUnsorted[unsortedIndex];
}

int CurveItem::unsortedIndex(int sortedIndex)
{
	sortPoints();
	return mUnsortedToSorted[sortedIndex];
}


void CurveItem::setPointOrderDirty()
{
	mPointOrderDirty = true;
}

const QVector<int>& CurveItem::sortPoints()
{
	if (!mPointOrderDirty)
		return mSortedToUnsorted;

	mSortedToUnsorted.clear();
	for (int i = 0, len = mCurve.pointCount(); i < len; i++)
		mSortedToUnsorted.append(i);

//	qSort(mSortedToUnsorted.begin(), mSortedToUnsorted.end(), [this](const QVariant& a, const QVariant& b)
//	{
//		bool ok;
//		qreal timeA = mCurve.data(a.toInt(&ok), datarole::POS).toPointF().x();
//		qreal timeB = mCurve.data(a.toInt(&ok), datarole::POS).toPointF().y();
//		return timeA < timeB;
//	});

	mUnsortedToSorted.resize(mSortedToUnsorted.size());
	for (int i = 0, len = mSortedToUnsorted.size(); i < len; i++)
	{
		int sortedIndex = mSortedToUnsorted[i];
		mUnsortedToSorted[sortedIndex] = i;
	}

	mPointOrderDirty = false;
	return mSortedToUnsorted;
}

QRectF CurveItem::boundingRect() const
{
	return childrenBoundingRect();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CurveView
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


CurveView::CurveView(QWidget* parent) : GridView(parent)
{
	setScene(&mCurveScene);
	mCurveScene.setSceneRect(-DEFAULT_SCENE_EXTENT, -DEFAULT_SCENE_EXTENT,
				 DEFAULT_SCENE_EXTENT * 2, DEFAULT_SCENE_EXTENT * 2);

	setPanZoomMode(PanMode::Parallax, ZoomMode::IgnoreAspectRatio);
	setFramePanZoomMode(PanMode::Parallax, ZoomMode::IgnoreAspectRatio);
	setGridIntervalDisplay(std::make_shared<FloatIntervalDisplay>(), std::make_shared<FloatIntervalDisplay>());

	// Flip y axis
	setVerticalFlipped(true);
	frameView(QRectF(0, 0, 1, 1), QMargins(10, 10, 10, 10));

}


void CurveView::mousePressEvent(QMouseEvent* event)
{
	mMousePressPos = event->pos();

	bool mCtrlHeld = event->modifiers() == Qt::ControlModifier;
	bool mShiftHeld = event->modifiers() == Qt::ShiftModifier;
	bool mAltHeld = event->modifiers() == Qt::AltModifier;
	bool lmb = event->buttons() == Qt::LeftButton;

	auto item = itemAt(event->pos());
	auto clickedHandle = dynamic_cast<HandleItem*>(item);
	auto clickedCurve = dynamic_cast<CurveSegmentItem*>(item);

	if (lmb)
	{
		if (!mCtrlHeld && !mShiftHeld)
		{
			if (clickedHandle)
			{
				setSelection({clickedHandle});
			} else if (clickedCurve)
			{
//				setSelection({clickedCurve});
			} else
			{
				startRubberBand(mMousePressPos);
			}
		} else if (mShiftHeld)
		{
			if (clickedHandle)
			{
//				addSelection({clickedHandle});
			}
		}

	}

//	GridView::mousePressEvent(event);
}

void CurveView::mouseMoveEvent(QMouseEvent* event)
{
	bool lmb = event->buttons() == Qt::LeftButton;
	bool mmb = event->buttons() == Qt::MiddleButton;
	if (!isRubberBandVisible())
	{
		if (lmb)
		{
			auto sceneDelta = mapToScene(event->pos()) - mapToScene(mLastMousePos);

			auto pointHandles = selectedItems<PointHandleItem>();
			auto tanHandles = selectedItems<TangentHandleItem>();
			movePointHandles(pointHandles, sceneDelta);
		}

		if (mmb)
		{
			auto sceneDelta = mapToScene(event->pos()) - mapToScene(mLastMousePos);

			auto pointHandles = selectedItems<PointHandleItem>();
			auto tanHandles = selectedItems<TangentHandleItem>();
			movePointHandles(pointHandles, sceneDelta);
		}
	}
	mLastMousePos = event->pos();
	GridView::mouseMoveEvent(event);
}

void CurveView::mouseReleaseEvent(QMouseEvent* event)
{
	if (isRubberBandVisible())
	{
		selectItemsInRubberband();
	}
	GridView::mouseReleaseEvent(event);
}

void CurveView::selectItemsInRubberband()
{
	auto rubberItems = items(rubberBandGeo());
	auto pointHandles = filter<PointHandleItem>(rubberItems);
	auto tanHandles = filter<TangentHandleItem>(rubberItems);
	auto handles = filter<HandleItem>(rubberItems);
	auto curves = filter<CurveItem>(handles);
	if (!pointHandles.isEmpty())
	{
		setSelection(pointHandles);
	} else if (!tanHandles.isEmpty())
	{
		setSelection(tanHandles);
	} else if (!curves.isEmpty())
	{
		setSelection(curves);
	} else
	{
		clearSelection();
	}
}

void CurveView::movePointHandles(const QList<PointHandleItem*>& handles, const QPointF& sceneDelta)
{
	QMap<CurveItem*, QMap<int, QPointF>> map;
	if (!handles.isEmpty())
	{
		for (auto handle : handles)
		{
			auto segmentItem = &handle->curveSegmentItem();
			auto curveItem = &segmentItem->curveItem();

			if (!map.contains(curveItem))
				map.insert(curveItem, QMap<int, QPointF>());

			int idx = curveItem->segmentIndex(*segmentItem);
			auto pos = handle->pos() + sceneDelta;

			map[curveItem][idx] = pos;
		}
	}

	for (auto& key : map.keys())
	{
		CurveItem* curveItem = key;
		curveItem->curve().movePoints(map[key]);
	}
}

void CurveView::moveTanHandles(const QList<TangentHandleItem*>& handles, const QPointF& sceneDelta)
{
	QMap<CurveItem*, QMap<int, QPointF>> map;
	if (!handles.isEmpty())
	{
		for (auto handle : handles)
		{
			auto segmentItem = &handle->curveSegmentItem();
			auto curveItem = &segmentItem->curveItem();

			if (!map.contains(curveItem))
				map.insert(curveItem, QMap<int, QPointF>());

			int idx = curveItem->segmentIndex(*segmentItem);
			auto pos = handle->pos() + sceneDelta;

			map[curveItem][idx] = pos;
		}
	}
//
//	for (auto& key : map.keys())
//	{
//		CurveItem* curveItem = key;
//		curveItem->curve().movePoints(map[key]);
//	}
}
void CurveView::setModel(AbstractCurveModel* model) {
	if (mModel == model)
		return;

	// clear out old model
	if (mModel)
	{
		disconnect(mModel, &AbstractCurveModel::curvesAdded, this, &CurveView::onCurvesAdded);
		disconnect(mModel, &AbstractCurveModel::curvesRemoved, this, &CurveView::onCurvesRemoved);
	}
	mCurveScene.clear();

	// register new model
	mModel = model;
	if (mModel)
	{
		connect(mModel, &AbstractCurveModel::curvesAdded, this, &CurveView::onCurvesAdded);
		connect(mModel, &AbstractCurveModel::curvesRemoved, this, &CurveView::onCurvesRemoved);
	}

	// populate with new data
	QList < int > indices;
	for (int i = 0, len = mModel->curveCount(); i < len; i++)
		indices << i;
	onCurvesAdded(indices);
}


void CurveView::onCurvesAdded(QList<int> indices)
{
	for (int index : indices)
	{
		auto curve = mModel->curve(index);
		auto curveItem = new CurveItem(nullptr, *curve);
		mCurveScene.addItem(curveItem);
		mCurveItems.insert(index, curveItem);
	}
}
void CurveView::onCurvesRemoved(QList<int> indices)
{
	auto sortedIndexes = reverseSort(indices);
	for (int index : sortedIndexes)
	{
		auto curveItem = mCurveItems.takeAt(index);
		delete curveItem;
	}
}