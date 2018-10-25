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

#define EPSILON 0.0001

bool fuzzyCompare(const QPointF& a, const QPointF& b)
{
	bool xEqual = qFuzzyCompare(a.x(), b.x());
	bool yEqual = qFuzzyCompare(a.y(), b.y());
	return xEqual && yEqual;
}

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

void limitOverhang(qreal& x0, qreal& x1, qreal& x2, qreal& x3)
{
	x1 = qMin(x1, x3);
	x2 = qMax(x2, x0);
}

// ensure bezier tangents aren't creating overhang (snapping) in the curve
void limitOverhangQPoints(QPointF& pa, QPointF& pb, QPointF& pc, QPointF& pd)
{
	qreal a = pa.x();
	qreal b = pb.x();
	qreal c = pc.x();
	qreal d = pd.x();
	qreal bb = b;
	qreal cc = c;

	limitOverhang(a, b, c, d);
	qreal rb = (b - a) / (bb - a);
	qreal rc = (c - d) / (cc - d);

	pb.setX(b);
	pc.setX(c);

	// we limited x, keep the point on the tangent line for c1 continuity
	pb.setY(((pb.y() - pa.y()) * rb) + pa.y());
	pc.setY(((pc.y() - pd.y()) * rc) + pd.y());
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

	return {otherTan.pos().x(), point.pos().y() + h};

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

TangentLineItem::TangentLineItem(QGraphicsItem& parent) : QGraphicsPathItem(&parent), mLimitItem(this)
{
	setZValue(ZDEPTH_HANDLE_LINES);
	setHighlighted(false);

	qreal limitExtent = 2;
	mLimitItem.setRect(-limitExtent, -limitExtent, limitExtent * 2, limitExtent * 2);
	mLimitItem.setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	mLimitItem.setPen(Qt::NoPen);
	mLimitItem.setVisible(false);
}

void TangentLineItem::setHighlighted(bool b)
{
	mHighlighted = b;
	QColor col = mHighlighted ? COL_TANLINE_SELECTED : COL_TANLINE;
	QBrush brush = QBrush(col);
	setPen(QPen(brush, 0));
	mLimitItem.setBrush(brush);
}

void TangentLineItem::setFromTo(const QPointF& a, const QPointF& b)
{
	QPainterPath p;
	p.moveTo(a);
	p.lineTo(b);
	setPath(p);
}

void TangentLineItem::showLimit(const QPointF& p)
{
	mLimitItem.setVisible(true);
	mLimitItem.setPos(p);
}

void TangentLineItem::hideLimit()
{
	mLimitItem.setVisible(false);
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
	if (mDrawQt)
	{
		painter->setPen(isSelected() ? mPenSelected : mPen);
		painter->drawPath(mPath);
	}

	if (mDrawDebug)
	{
		painter->setPen(isSelected() ? mPenSelected : mPen);
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

		auto a = pos;
		auto b = c1;
		auto c = c2;
		auto d = nextPos;

		limitOverhangQPoints(a, b, c, d);
		if (!fuzzyCompare(c1, b))
			mOutTanLine.showLimit(b);
		else
			mOutTanLine.hideLimit();

		if (!fuzzyCompare(c2, c))
			nextSeg->inTanLine().showLimit(c);
		else
			nextSeg->inTanLine().hideLimit();

		// Draw debug path
		mDebugPath.moveTo(a);
		for (int i = 1; i <= mSampleCount; i++)
		{
			qreal t = i / (qreal) mSampleCount;
			qreal x = a.x() + (d.x() - a.x()) * t;
			auto p = evalCurveSegment({a, b, c, d}, x);
			mDebugPath.lineTo(x, p);
		}

		// Use Qt's cubic curve
		mPath.moveTo(a);
		mPath.cubicTo(b, c, d);
	}

	QGraphicsItem::update();

	setPointsEmitItemChanges(true);
}

void CurveSegmentItem::onHandleMoved(HandleItem* handle)
{
	handleMoved(handle);
}

void CurveSegmentItem::setPointsEmitItemChanges(bool b)
{
	mPointHandle.setEmitItemChanged(b);
	mInTanHandle.setEmitItemChanged(b);
	mOutTanHandle.setEmitItemChanged(b);
}

void CurveSegmentItem::onTanHandleSelected(HandleItem* handle)
{
	if (handle == &mInTanHandle)
		mInTanLine.setHighlighted(handle->isSelected());
	else if (handle == &mOutTanHandle)
		mOutTanLine.setHighlighted(handle->isSelected());
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

	setRenderHint(QPainter::Antialiasing, true);
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
	auto clickedPointHandle = dynamic_cast<PointHandleItem*>(item);
	auto clickedTanHandle = dynamic_cast<TangentHandleItem*>(item);
	auto clickedCurve = dynamic_cast<CurveSegmentItem*>(item);

	if (lmb)
	{
		if (!ctrlHeld && !shiftHeld)
		{
			if (clickedPointHandle)
			{
				if (!clickedPointHandle->isSelected())
				{
					clearSelection();
					selectPointHandles({clickedPointHandle});
				}
				mInteractMode = DragPoints;
			}
			else if (clickedTanHandle)
			{
				if (!clickedTanHandle->isSelected())
				{
					for (TangentHandleItem* tan : filterT<TangentHandleItem>(scene()->selectedItems()))
						if (tan != clickedTanHandle)
							tan->setSelected(false);

					for (PointHandleItem* handle : filterT<PointHandleItem>(scene()->selectedItems()))
						handle->setSelected(false);

					addSelection({clickedTanHandle});
				}
				mInteractMode = DragPoints;
			}
			else
			{
				startRubberBand(mMousePressPos);
				mInteractMode = Rubberband;
			}
		}
		else if (shiftHeld)
		{
			if (clickedPointHandle)
			{
				selectPointHandles({clickedPointHandle});
			}
			else if (clickedTanHandle)
			{
//				for (TangentHandleItem* tan : filterT<TangentHandleItem>(scene()->selectedItems()))
//					if (tan != clickedTanHandle)
//						tan->setSelected(false);

				for (PointHandleItem* handle : filterT<PointHandleItem>(scene()->selectedItems()))
					handle->setSelected(false);

				addSelection({clickedTanHandle});
				mInteractMode = SelectAdd;
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

		auto pointHandles = filterT<PointHandleItem>(rubberItems);
		auto tanHandles = filter<TangentHandleItem>(rubberItems);

		if (!pointHandles.isEmpty())
		{
			if (mInteractMode != RubberbandAdd)
				clearSelection();
			selectPointHandles(pointHandles);
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
	else if (mInteractMode == SelectAdd)
	{

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
	for (TangentHandleItem* tan : tans)
	{
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
		{
			auto p = pos - tangent->pointHandle().pos();
			if (p.x() > -EPSILON)
				p.setX(-EPSILON);
			positions[0][idx] = p;
		}
		else
		{
			auto p = pos - tangent->pointHandle().pos();
			if (p.x() < EPSILON)
				p.setX(EPSILON);
			positions[1][idx] = p;
		}
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


void CurveView::selectPointHandles(const QList<PointHandleItem*>& pointHandles)
{
	QList < QGraphicsItem * > graphicsItems;
	for (auto p : pointHandles)
	{
		p->curveSegmentItem().setTangentsVisible(true);
		graphicsItems.append(p);
	}

	auto selectedPoints = filterT<PointHandleItem>(scene()->selectedItems());
	for (PointHandleItem* pt : filterT<PointHandleItem>(scene()->items()))
		if (!pointHandles.contains(pt) && !selectedPoints.contains(pt))
			pt->curveSegmentItem().setTangentsVisible(false);

	addSelection(graphicsItems);
}
