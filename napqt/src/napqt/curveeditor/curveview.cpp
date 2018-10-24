#include <napqt/timeline/eventitem.h>
#include "curveview.h"
#include "curvemath.h"
#include <QtDebug>
#include <QMap>
#include <QList>
#include <cassert>
#include <QVector>
#include <QtGui/QtGui>

using namespace napqt;

#define DEFAULT_SCENE_EXTENT 1000

#define COL_POINTHANDLE_FILL            "#000"
#define COL_POINTHANDLE_FILL_SELECTED    "#FFF"
#define COL_POINTHANDLE_LINE            "#000"
#define COL_POINTHANDLE_LINE_SELECTED    "#000"
#define COL_TANHANDLE_FILL                "#333"
#define COL_TANHANDLE_FILL_SELECTED    "#0F0"
#define COL_TANHANDLE_LINE                "#0FF"
#define COL_TANHANDLE_LINE_SELECTED    "#0FF"
#define COL_TANLINE                    "#888"
#define COL_TANLINE_SELECTED            "#F0F"

#define ZDEPTH_HANDLES 15000
#define ZDEPTH_HANDLE_LINES 10000
#define ZDEPTH_CURVES 5000
#define ZDEPTH_AUXILIARY 0

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

// For aligned tangents, returns the vector for the 'unmoved' (other) handle
QPointF alignedOppositeTangentVector(TangentHandleItem& handle)
{
	// align the angle, but leave the x position
	// This is the same as Blender's behavior, which is view scale independent.
	// Maya also matches the vector length, which is view scale dependent.
	const auto& point = handle.pointHandle();
	const auto& otherTan = handle.oppositeTanHandle();

	auto localTanPos = handle.pos() - point.pos();
	auto otherTanPos = otherTan.pos() - point.pos();

	auto a = qAtan2(-localTanPos.y(), -localTanPos.x());
	auto x = otherTanPos.x();
	auto h = qTan(a) * x;

	return { otherTan.pos().x(), point.pos().y() + h};

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
	setZValue(ZDEPTH_HANDLES);

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
	}
	else if (change == QGraphicsItem::ItemSelectedHasChanged)
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
	painter->fillPath(mPath, painter->brush());
	painter->drawPath(mPath);
}

CurveSegmentItem& HandleItem::curveSegmentItem()
{
	return *dynamic_cast<CurveSegmentItem*>(parentItem());
}

const CurveSegmentItem& HandleItem::curveSegmentItem() const
{
	return *dynamic_cast<CurveSegmentItem*>(parentItem());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PointHandleItem
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PointHandleItem::PointHandleItem(CurveSegmentItem& parent) : HandleItem(parent)
{
	mPen = QPen(Qt::NoPen);
	mPenSelected = QPen(QColor(COL_POINTHANDLE_LINE), 0, Qt::SolidLine);
	mBrush = QBrush(COL_POINTHANDLE_FILL);
	mBrushSelected = QBrush(COL_POINTHANDLE_FILL_SELECTED);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TangentHandleItem
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TangentHandleItem::TangentHandleItem(CurveSegmentItem& parent) : HandleItem(parent)
{
	mPen = QPen(Qt::NoPen);
	mPenSelected = QPen(QColor(COL_TANHANDLE_LINE), 0, Qt::SolidLine);
	mBrush = QBrush(COL_TANHANDLE_FILL);
	mBrushSelected = QBrush(COL_TANHANDLE_FILL_SELECTED);
}

PointHandleItem& TangentHandleItem::pointHandle()
{
	return curveSegmentItem().pointHandle();
}

TangentHandleItem& TangentHandleItem::oppositeTanHandle()
{
	if (isInTangent())
		return curveSegmentItem().outTanHandle();
	return curveSegmentItem().inTanHandle();
}

bool TangentHandleItem::isInTangent()
{
	return &curveSegmentItem().inTanHandle() == this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TangentLineItem
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LineItem::LineItem(QGraphicsItem& parent) : QGraphicsPathItem(&parent)
{
	setZValue(ZDEPTH_HANDLE_LINES);
	setHighlighted(false);
}

void LineItem::setHighlighted(bool b)
{
	mHighlighted = b;
	QColor col = mHighlighted ? COL_TANLINE_SELECTED : COL_TANLINE;
	setPen(QPen(QBrush(col), 0));
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
	setZValue(ZDEPTH_CURVES);
	setFlag(QGraphicsItem::ItemIsSelectable, false);

	mPen = QPen(Qt::blue, 0);
	mPenDebug = QPen(Qt::gray, 0);
	mPenSelected = QPen(Qt::red, 0);

	connect(&mPointHandle, &HandleItem::moved, this, &CurveSegmentItem::onHandleMoved);
	connect(&mInTanHandle, &HandleItem::moved, this, &CurveSegmentItem::onHandleMoved);
	connect(&mOutTanHandle, &HandleItem::moved, this, &CurveSegmentItem::onHandleMoved);
	connect(&mPointHandle, &HandleItem::selected, this, &CurveSegmentItem::updateHandleVisibility);
	connect(&mPointHandle, &HandleItem::selected, this, &CurveSegmentItem::updateHandleVisibility);

	connect(&mInTanHandle, &HandleItem::selected, this, &CurveSegmentItem::onTanHandleSelected);
	connect(&mOutTanHandle, &HandleItem::selected, this, &CurveSegmentItem::onTanHandleSelected);

	setInTanVisible(false);
	setOutTanVisible(false);
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

void CurveSegmentItem::setInTanVisible(bool b)
{
	mInTanHandle.setVisible(b);
	mInTanLine.setVisible(b);
}

void CurveSegmentItem::setOutTanVisible(bool b)
{
	mOutTanHandle.setVisible(b);
	mOutTanLine.setVisible(b);
}

void CurveSegmentItem::setTangentsVisible(bool b)
{
	setInTanVisible(b && !isFirstPoint());
	setOutTanVisible(b && !isLastPoint());
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

	setPointsEmitItemChanges(false);

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

	setPointsEmitItemChanges(true);
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

void CurveSegmentItem::updateHandleVisibility()
{

//	auto& curve = curveItem().curve();
//	int segCount = curve.pointCount();
//	int idx = index();
//	int orderedIdx = orderedIndex();
//
//	bool isLast = orderedIdx == segCount - 1;
//	bool isFirst = orderedIdx == 0;
//
//	bool s = mPointHandle.isSelected();
//	bool anyTanSelected = mInTanHandle.isSelected() || mOutTanHandle.isSelected();
//
//	bool inTanVisible = isInTanVisible();
//	bool outTanVisible = isOutTanVisible();
//
//	mInTanLine.setVisible(inTanVisible);
//	mInTanHandle.setVisible(inTanVisible);
//	mOutTanLine.setVisible(outTanVisible);
//	mOutTanHandle.setVisible(outTanVisible);
}

void CurveSegmentItem::onTanHandleSelected(HandleItem* handle)
{
	if (handle == &mInTanHandle)
		mInTanLine.setHighlighted(handle->isSelected());
	else if (handle == &mOutTanHandle)
		mOutTanLine.setHighlighted(handle->isSelected());
}


bool CurveSegmentItem::isInTanVisible()
{
	int orderedIdx = orderedIndex();
	bool isFirst = orderedIdx == 0;

	if (isFirst)
		return false;

	if (mInTanHandle.isSelected())
		return true;

	if (mOutTanHandle.isSelected())
		return true;

	if (mPointHandle.isSelected())
		return true;

	return false;
}

bool CurveSegmentItem::isOutTanVisible()
{
	int segCount = curveItem().curve().pointCount();
	int orderedIdx = orderedIndex();
	bool isLast = orderedIdx == segCount - 1;

	if (isLast)
		return false;

	if (mInTanHandle.isSelected())
		return true;

	if (mOutTanHandle.isSelected())
		return true;

	if (mPointHandle.isSelected())
		return true;

	return false;
}

bool CurveSegmentItem::isLastPoint()
{
	return curveItem().isLastPoint(index());
}

bool CurveSegmentItem::isFirstPoint()
{
	return curveItem().isFirstPoint(index());
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
	for (int idx : reverseSort(indices))
		updateSegmentFromPoint(idx);
}

void CurveItem::updateSegmentFromPoint(int i)
{
	CurveSegmentItem& seg = *mSegments[i];
	seg.updateGeometry();
	// this handle controls the shape of the previous point too
	// TODO: this changes when using higher order bsplines
	if (!isFirstPoint(segmentIndex(seg)))
		prevSegment(seg)->updateGeometry();
}

void CurveItem::updateAllSegments()
{
	for (int i = 0, len = mCurve.pointCount(); i < len; i++)
		updateSegmentFromPoint(i);
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
	int unsortedIndex = segmentIndex(seg);
	return mSegments[nextSegIndex(unsortedIndex)];
}

CurveSegmentItem* CurveItem::prevSegment(const CurveSegmentItem& seg)
{
	int unsortedIndex = segmentIndex(seg);
	return mSegments[prevSegIndex(unsortedIndex)];
}

int CurveItem::nextSegIndex(int idx)
{
	sortPoints();
	int sortedIndex = mUnsortedToSorted[idx];
	int outSortedIndex = sortedIndex + 1;
	int outidx = mSortedToUnsorted[outSortedIndex];
	return outidx;
}

int CurveItem::prevSegIndex(int idx)
{
	sortPoints();
	int sortedIndex = mUnsortedToSorted[idx];
	int outSortedIndex = sortedIndex - 1;
	int outidx = mSortedToUnsorted[outSortedIndex];
	return outidx;
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
	QList < int > idx;
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
	return sortedIndex(segmentIndex(seg));
}

int CurveItem::sortedIndex(int unsortedIndex)
{
	sortPoints();
	return mUnsortedToSorted[unsortedIndex];
}

int CurveItem::unsortedIndex(int sortedIndex)
{
	sortPoints();
	return mSortedToUnsorted[sortedIndex];
}


void CurveItem::setPointOrderDirty()
{
	mPointOrderDirty = true;
}

const QVector<int>& CurveItem::sortPoints()
{
	if (!mPointOrderDirty)
		return mSortedToUnsorted;

	int len = mCurve.pointCount();

	mSortedToUnsorted.clear();
	for (int i = 0; i < len; i++)
		mSortedToUnsorted.append(i);

	std::sort(mSortedToUnsorted.begin(), mSortedToUnsorted.end(), [this](const QVariant& a, const QVariant& b)
	{
		bool ok;
		qreal timeA = mCurve.data(a.toInt(&ok), datarole::POS).toPointF().x();
		qreal timeB = mCurve.data(b.toInt(&ok), datarole::POS).toPointF().x();
		return timeA < timeB;
	});

	mUnsortedToSorted.resize(len);
	for (int i = 0; i < len; i++)
	{
		int sortedIndex = mSortedToUnsorted[i];
		mUnsortedToSorted[sortedIndex] = i;
	}

	mPointOrderDirty = false;
	updateAllSegments();
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

	bool ctrlHeld = event->modifiers() == Qt::ControlModifier;
	bool shiftHeld = event->modifiers() == Qt::ShiftModifier;
	bool altHeld = event->modifiers() == Qt::AltModifier;
	bool lmb = event->buttons() == Qt::LeftButton;
	bool mmb = event->buttons() == Qt::MiddleButton;

	auto item = itemAt(event->pos());
	auto clickedHandle = dynamic_cast<HandleItem*>(item);
	auto clickedCurve = dynamic_cast<CurveSegmentItem*>(item);

	if (lmb)
	{
		if (!ctrlHeld && !shiftHeld)
		{
			if (clickedHandle)
			{
				setSelection({clickedHandle});
			}
			else
			{
				startRubberBand(mMousePressPos);
				mInteractMode = Rubberband;
			}
		}
		else if (shiftHeld)
		{
			if (clickedHandle)
			{
//				addSelection({clickedHandle});
			}
			else
			{
				startRubberBand(mMousePressPos);
				mInteractMode = RubberbandAdd;
			}
		}
	}
	else if (mmb)
	{
		if (altHeld)
		{
			mInteractMode = Pan;
		}
		else
		{
			mInteractMode = DragPoints;
		}
	}

//	GridView::mousePressEvent(event);
}

void CurveView::mouseMoveEvent(QMouseEvent* event)
{
	bool ctrlHeld = event->modifiers() == Qt::ControlModifier;
	bool shiftHeld = event->modifiers() == Qt::ShiftModifier;
	bool altHeld = event->modifiers() == Qt::AltModifier;

	bool lmb = event->buttons() == Qt::LeftButton;
	bool mmb = event->buttons() == Qt::MiddleButton;

	if (mInteractMode == DragPoints)
	{
		auto sceneDelta = mapToScene(event->pos()) - mapToScene(mLastMousePos);

		auto pointHandles = selectedItems<PointHandleItem>();
		auto tanHandles = selectedItems<TangentHandleItem>();
		if (!pointHandles.isEmpty())
		{
			movePointHandles(pointHandles, sceneDelta);
		}
		else if (!tanHandles.isEmpty())
		{
			moveTanHandles(tanHandles, sceneDelta);
		}
	}
	mLastMousePos = event->pos();
	GridView::mouseMoveEvent(event);
}

void CurveView::mouseReleaseEvent(QMouseEvent* event)
{
	if (mInteractMode == Rubberband || mInteractMode == RubberbandAdd)
	{
		hideRubberBand();
		auto rubberItems = items(rubberBandGeo());

		auto pointHandles = filter<PointHandleItem>(rubberItems);
		auto tanHandles = filter<TangentHandleItem>(rubberItems);

		if (!pointHandles.isEmpty())
		{

			if (mInteractMode != RubberbandAdd)
				clearSelection();

			auto selectedPoints = filterT<PointHandleItem>(scene()->selectedItems());

			for (PointHandleItem* pt : filterT<PointHandleItem>(scene()->items()))
				if (!selectedPoints.contains(pt))
					pt->curveSegmentItem().setTangentsVisible(false);

			addSelection(pointHandles);
			for (auto handle : pointHandles)
				dynamic_cast<PointHandleItem*>(handle)->curveSegmentItem().setTangentsVisible(true);
		}
		else if (!tanHandles.isEmpty())
		{
			for (PointHandleItem* handle : filterT<PointHandleItem>(scene()->selectedItems()))
				handle->setSelected(false);

			if (mInteractMode == RubberbandAdd)
			{
				addSelection(tanHandles);
			}
			else
			{
				clearSelection();
				addSelection(tanHandles);
			}
		}
		else
		{
			clearSelection();
			for (auto handle : filterT<PointHandleItem>(items()))
			{
				handle->curveSegmentItem().setTangentsVisible(false);
			}
		}
	}
	mInteractMode = None;
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

void CurveView::moveTanHandles(const QList<TangentHandleItem*>& tans, const QPointF& sceneDelta)
{
	if (tans.isEmpty())
		return;

	QList < TangentHandleItem * > movedTangents;
	for (TangentHandleItem* tan : tans) {
		if (tans.contains(&tan->oppositeTanHandle()))
			continue;
		movedTangents << tan;
	}

	QMap<CurveItem*, QList<QMap<int, QPointF>>> tanMap;
	for (TangentHandleItem* tangent : movedTangents)
	{
		TangentHandleItem& otherTan = tangent->oppositeTanHandle();

		auto segmentItem = &tangent->curveSegmentItem();
		auto curveItem = &segmentItem->curveItem();
		int idx = curveItem->segmentIndex(*segmentItem);
		auto pos = tangent->pos() + sceneDelta;

		if (!tanMap.contains(curveItem))
		{
			QList < QMap<int, QPointF>>
			ls;
			ls << QMap<int, QPointF>();
			ls << QMap<int, QPointF>();
			tanMap.insert(curveItem, ls);
		}

		auto& positions = tanMap[curveItem];

		if (tangent->isInTangent())
			positions[0][idx] = pos - tangent->pointHandle().pos();
		else
			positions[1][idx] = pos - tangent->pointHandle().pos();

		// Move opposite tangent
		auto otherPos = alignedOppositeTangentVector(*tangent);

		if (otherTan.isInTangent())
			positions[0][idx] = otherPos - otherTan.pointHandle().pos();
		else
			positions[1][idx] = otherPos - otherTan.pointHandle().pos();
	}

	for (auto& curveItem : tanMap.keys())
	{
		auto tanPosList = tanMap[curveItem];
		curveItem->curve().moveTangents(tanPosList[0], tanPosList[1]);
	}

}
void CurveView::setModel(AbstractCurveModel* model)
{
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


const QList<TangentHandleItem*> CurveView::tanHandles()
{
	return filterT<TangentHandleItem>(scene()->items());
}

const QList<PointHandleItem*> CurveView::pointHandles()
{
	return filterT<PointHandleItem>(scene()->items());
}
