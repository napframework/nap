/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "curveview.h"
#include "curvemath.h"
#include "napqt-resources.h"

#include <QMenu>
#include <QtDebug>
#include <QtGui>
#include <QPainter>
#include <QList>
#include <QToolButton>

#include <napqt/qtutils.h>
#include <napqt/separator.h>

using namespace nap::qt;

#define DEFAULT_SCENE_EXTENT 1000

#define COL_POINTHANDLE_FILL            "#000"
#define COL_POINTHANDLE_FILL_SELECTED    "#FFF"
#define COL_POINTHANDLE_LINE            "#000"
#define COL_POINTHANDLE_LINE_SELECTED    "#000"
#define COL_TANHANDLE_FILL                "#333"
#define COL_TANHANDLE_FILL_SELECTED    "#FFF"
#define COL_TANHANDLE_LINE                "#0FF"
#define COL_TANHANDLE_LINE_SELECTED    "#0FF"
#define COL_TANLINE                    "#888"
#define COL_TANLINE_SELECTED            "#DDD"
#define COL_CURVE_HIGHLIGHT                "#F80"
#define COL_CURVE_SELECTED                "#FFF"

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

qreal length(const QPointF& p)
{
	return qSqrt(p.x() * p.x() + p.y() * p.y());
}

QPointF normalize(const QPointF& p)
{
	qreal mag = length(p);
	if (mag == 0)
		mag = EPSILON;
	return {p.x() / mag, p.y() / mag};
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

HandleItem::HandleItem(CurveSegmentItem& parent) : QObject(), QGraphicsPathItem(&parent)
{
	setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	setFlag(QGraphicsItem::ItemIsMovable, true);
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);
	setZValue(ZDEPTH_HANDLES);

	updateRect();
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
	QPainterPath path;
	auto ext = isSelected() ? mExtentSelectd : mExtent;
	mRect.setCoords(-ext, -ext, ext * 2, ext * 2);
	path.addRect(mRect);
	setPath(path);
	mShape = QPainterPath();
	mShape.addRect(-mShapeExtent, -mShapeExtent, mShapeExtent * 2, mShapeExtent * 2);
}

QPainterPath HandleItem::shape() const
{
	return mShape;
}

void HandleItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setPen(isSelected() ? mPenSelected : mPen);
	painter->setBrush(isSelected() ? mBrushSelected : mBrush);
	painter->fillPath(path(), painter->brush());
	painter->drawPath(path());
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
void TangentHandleItem::updateRect()
{
	QPainterPath path;
	auto ext = isSelected() ? mExtentSelectd : mExtent;
	mRect.setCoords(-ext, -ext, ext * 2, ext * 2);

	const auto& curve = curveSegmentItem().curveItem().curve();
	int idx = curveSegmentItem().index();

	if (curve.tangentsAligned(idx))
	{
		path.addEllipse(mRect);
	}
	else
	{
		path.addRect(mRect);
	}
	setPath(path);
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
		: QObject(), QGraphicsPathItem(&curveItem),
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
	return path().boundingRect().united(childrenBoundingRect());
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
	bool bb = !isFirstPoint();
	if (bb)
	{
		int prevIndex = curveItem().prevSegIndex(index());
		const auto& prevInterp = curveItem().curve().interpolation(prevIndex);
		bb = bb && prevInterp == AbstractCurve::InterpType::Bezier;
	}
	setInTanVisible(bb && b);

	const auto& interp = curveItem().curve().interpolation(index());
	setOutTanVisible(b && !isLastPoint() && interp == AbstractCurve::InterpType::Bezier);
}

void CurveSegmentItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setPen(isSelected() ? mPenSelected : mPen);
	painter->drawPath(path());
}

int CurveSegmentItem::index() const
{
	return curveItem().segmentIndex(*this);
}

int CurveSegmentItem::orderedIndex()
{
	return curveItem().sortedIndex(*this);
}

QPainterPath CurveSegmentItem::shape() const
{
	return path();
}

void CurveSegmentItem::updateGeometry()
{
	setPointsEmitItemChanges(false);

	auto& curve = curveItem().curve();

	mPen.setColor(curve.color());

	int segCount = curve.pointCount();
	int idx = index();
	int orderedIdx = orderedIndex();

	bool isLast = orderedIdx == segCount - 1;

	const auto pos = curve.pos(idx);
	const auto inTan = curve.inTangent(idx);
	const auto outTan = curve.outTangent(idx);
	const auto interp = curve.interpolation(idx);


	auto c1 = pos + outTan;
	auto inTanPos = pos + inTan;
	auto outTanPos = pos + outTan;

	mPointHandle.setPos(pos);
	mInTanHandle.setPos(inTanPos);
	mInTanHandle.updateRect();
	mOutTanHandle.setPos(outTanPos);
	mOutTanHandle.updateRect();

	mInTanLine.setFromTo(pos, inTanPos);
	mOutTanLine.setFromTo(pos, outTanPos);

	if (mPointHandle.isSelected())
		setTangentsVisible(true);

	QPainterPath path;

	if (!isLast)
	{
		// Handle anything that needs the next segment/point
		auto nextSeg = curveItem().nextSegment(*this);
		auto nextIdx = curveItem().segmentIndex(*nextSeg);
		auto nextInTan = curve.inTangent(nextIdx);
		auto nextPos = curve.pos(nextIdx);
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
		if (interp == AbstractCurve::InterpType::Stepped)
		{
			// Interpolated stepped looks a little weird when interpolated (ramp at last segment) at lower
			// sample rate. Draw as if samples are infinitely small.
			path.moveTo(a);
			path.lineTo(d.x(), a.y());
			path.lineTo(d);
		}
		else
		{
			path.moveTo(a);
			for (int i = 1; i <= mSampleCount; i++)
			{
				qreal t = i / (qreal) mSampleCount;
				qreal x = a.x() + (d.x() - a.x()) * t;
				qreal v = curve.evaluate(x);
				path.lineTo(x, v);
			}
			path.lineTo(d.x(), d.y());
		}
	}
	setPath(path);
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

	QList<int> indices;
	for (int i = 0, len = curve.pointCount(); i < len; i++)
		indices << i;
	onPointsAdded(indices);
}

void CurveItem::onPointsChanged(QList<int> indices)
{
	setPointOrderDirty();
	updateAllSegments();
}

void CurveItem::updateSegmentFromPoint(int i)
{
	CurveSegmentItem& seg = *mSegments[i];

	seg.updateGeometry();
	// this handle controls the shape of the previous point too
	// TODO: this changes when using higher order bsplines
	if (!isFirstPoint(segmentIndex(seg)))
		prevSegment(seg)->updateGeometry();
	update();
}

void CurveItem::updateAllSegments()
{
	for (int i = 0, len = mCurve.pointCount(); i < len; i++)
		updateSegmentFromPoint(i);
	update();
	if (scene())
	{
		scene()->update();
	}
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
	QList<CurveSegmentItem*> segments;
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
	updateAllSegments();
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

	std::sort(mSortedToUnsorted.begin(), mSortedToUnsorted.end(), [this](const int& a, const int& b)
	{
		return mCurve.pos(a).x() < mCurve.pos(b).x();
	});

	mUnsortedToSorted.resize(len);
	for (int i = 0; i < len; i++)
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
	frameView(QRectF(0, 0, 1, 1));

	setRenderHint(QPainter::Antialiasing, true);
	setContextMenuPolicy(Qt::NoContextMenu);

	initActions();

	connect(&mCurveScene, &QGraphicsScene::selectionChanged, [this]() { selectionChanged(selectedPoints()); });
}


void CurveView::mousePressEvent(QMouseEvent* event)
{
	mMousePressPos = event->pos();

	bool ctrlHeld = event->modifiers() == Qt::ControlModifier;
	bool shiftHeld = event->modifiers() == Qt::ShiftModifier;
	bool altHeld = event->modifiers() == Qt::AltModifier;
	bool lmb = event->buttons() == Qt::LeftButton;
	bool mmb = event->buttons() == Qt::MiddleButton;
	bool rmb = event->buttons() == Qt::RightButton;

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
			else if (!altHeld)
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
		else if (ctrlHeld && model())
		{
			auto scenePos = mapToScene(event->pos());
			auto curve = closestCurve(event->pos());
			if (curve)
				curve->addPoint(scenePos.x(), scenePos.y());
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
	else if (rmb)
	{
		if (!shiftHeld && !ctrlHeld && !altHeld)
			onCustomContextMenuRequested(event->pos());
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

	auto scenePos = mapToScene(event->pos());

	if (mInteractMode == DragPoints)
	{
		auto sceneDelta = scenePos - mapToScene(mLastMousePos);

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
	else if (mInteractMode == DragPoints)
	{
		commitPointEditChanges(true);
	}

	mInteractMode = None;
}


void CurveView::movePointHandles(const QList<PointHandleItem*>& handles, const QPointF& sceneDelta)
{
	mPointEditMap.clear();
	if (!handles.isEmpty())
	{
		for (auto handle : handles)
		{
			auto segmentItem = &handle->curveSegmentItem();
			auto curveItem = &segmentItem->curveItem();

			if (!mPointEditMap.contains(curveItem))
				mPointEditMap.insert(curveItem, QMap<int, QPointF>());

			int idx = curveItem->segmentIndex(*segmentItem);
			auto pos = handle->pos() + sceneDelta;

			mPointEditMap[curveItem][idx] = pos;
		}
	}

	commitPointEditChanges(false);
}

void CurveView::moveTanHandles(const QList<TangentHandleItem*>& tans, const QPointF& sceneDelta)
{
	if (tans.isEmpty())
		return;

	QList<TangentHandleItem*> movedTangents;
	for (TangentHandleItem* tan : tans)
	{
		if (tans.contains(&tan->oppositeTanHandle()))
			continue;
		movedTangents << tan;
	}

	mTangentEditMap.clear();
	for (TangentHandleItem* tangent : movedTangents)
	{
		TangentHandleItem& otherTan = tangent->oppositeTanHandle();

		auto segmentItem = &tangent->curveSegmentItem();
		auto curveItem = &segmentItem->curveItem();
		int idx = curveItem->segmentIndex(*segmentItem);
		auto pos = tangent->pos() + sceneDelta;

		if (!mTangentEditMap.contains(curveItem))
		{
			QList<QMap<int, QPointF>> ls;
			ls << QMap<int, QPointF>();
			ls << QMap<int, QPointF>();
			mTangentEditMap.insert(curveItem, ls);
		}

		auto& positions = mTangentEditMap[curveItem];

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
		if (curveItem->curve().tangentsAligned(idx))
		{
			auto otherPos = alignedOppositeTangentVector(*tangent);

			if (otherTan.isInTangent())
				positions[0][idx] = otherPos - otherTan.pointHandle().pos();
			else
				positions[1][idx] = otherPos - otherTan.pointHandle().pos();
		}
	}

	commitPointEditChanges(false);
}
void CurveView::setModel(AbstractCurveModel* model)
{
	if (mModel == model)
		return;

	// clear out old model
	if (mModel)
	{
		disconnect(mModel, &AbstractCurveModel::curvesInserted, this, &CurveView::onCurvesAdded);
		disconnect(mModel, &AbstractCurveModel::curvesRemoved, this, &CurveView::onCurvesRemoved);
	}
	mCurveScene.clear();

	// register new model
	mModel = model;
	if (mModel)
	{
		connect(mModel, &AbstractCurveModel::curvesInserted, this, &CurveView::onCurvesAdded);
		connect(mModel, &AbstractCurveModel::curvesRemoved, this, &CurveView::onCurvesRemoved);

		// populate with new data
		QList<int> indices;
		for (int i = 0, len = mModel->curveCount(); i < len; i++)
			indices << i;
		onCurvesAdded(indices);
	}
}

void CurveView::initActions()
{
	mDeleteAction.setText("Delete");
	mDeleteAction.setShortcut(QKeySequence::Delete);
	mDeleteAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(&mDeleteAction, &QAction::triggered, this, &CurveView::deleteSelectedItems);
	addAction(&mDeleteAction);

	mInterpBezierAction.setText("Bezier");
	mInterpBezierAction.setIcon(QIcon(nap::qt::QRC_ICONS_CURVEINTERP_BEZIER));
	connect(&mInterpBezierAction, &QAction::triggered,
			[this]() { setSelectedPointInterps(AbstractCurve::InterpType::Bezier); });

	mInterpLinearAction.setText("Linear");
	mInterpLinearAction.setIcon(QIcon(nap::qt::QRC_ICONS_CURVEINTERP_LINEAR));
	connect(&mInterpLinearAction, &QAction::triggered,
			[this]() { setSelectedPointInterps(AbstractCurve::InterpType::Linear); });

	mInterpSteppedAction.setText("Stepped");
	QIcon icon(nap::qt::QRC_ICONS_CURVEINTERP_STEPPED);
	mInterpSteppedAction.setIcon(icon);
	connect(&mInterpSteppedAction, &QAction::triggered,
			[this]() { setSelectedPointInterps(AbstractCurve::InterpType::Stepped); });

	mSetTangentsAlignedAction.setText("Aligned");
	mSetTangentsAlignedAction.setIcon(QIcon(nap::qt::QRC_ICONS_TANGENTS_ALIGNED));
	connect(&mSetTangentsAlignedAction, &QAction::triggered,
			[this]() { setSelectedTangentsAligned(true); });

	mSetTangentsBrokenAction.setText("Broken");
	mSetTangentsBrokenAction.setIcon(QIcon(nap::qt::QRC_ICONS_TANGENTS_BROKEN));
	connect(&mSetTangentsBrokenAction, &QAction::triggered,
			[this]() { setSelectedTangentsAligned(false); });

	mFlattenTangentsAction.setText("Flatten Tangents");
	mFlattenTangentsAction.setIcon(QIcon(nap::qt::QRC_ICONS_TANGENTS_FLAT));
	connect(&mFlattenTangentsAction, &QAction::triggered,
			[this]() { setSelectedTangentsFlat(); });

	mFrameViewAction.setText("Frame Selection");
	mFrameViewAction.setIcon(QIcon(nap::qt::QRC_ICONS_FRAME_SELECTION));
	connect(&mFrameViewAction, &QAction::triggered,
			[this]() { frameSelected(); });

}

void CurveView::deleteSelectedItems()
{
	QMap<int, QList<int>> toDelete;
	for (PointHandleItem* pointHandle : selectedItems<PointHandleItem>())
	{
		auto& curveItem = pointHandle->curveSegmentItem().curveItem();
		int curveIndex = model()->curveIndex(&curveItem.curve());
		int pointIndex = pointHandle->curveSegmentItem().index();

		if (!toDelete.contains(curveIndex))
			toDelete[curveIndex] = QList<int>();

		toDelete[curveIndex].append(pointIndex);
	}

	for (auto it = toDelete.begin(); it != toDelete.end(); ++it)
		model()->curve(it.key())->removePoints(it.value());

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
	QList<QGraphicsItem*> graphicsItems;
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

AbstractCurve* CurveView::closestCurve(const QPointF& pos)
{
	if (!model())
		return nullptr;

	if (model()->curveCount() < 1)
		return nullptr;

	// First, attempt to get the curve under the mouse cursor
	for (auto item : items(pos.x(), pos.y()))
	{
		auto segment = dynamic_cast<CurveSegmentItem*>(item);
		if (segment)
			return &segment->curveItem().curve();
	}

	// No curve found at location, find closest point on curves
	AbstractCurve* closestCurve = nullptr;

	qreal closestDist = std::numeric_limits<qreal>::max();
	for (int i = 0, len = model()->curveCount(); i < len; i++)
	{
		const auto curve = model()->curve(i);
		if (curve->pointCount() == 0)
			continue;

		auto p = closestPointOnCurve(*curve, pos);
		auto dist = length(p - pos);
		if (dist < closestDist)
		{
			closestDist = dist;
			closestCurve = curve;
		}
	}
	if (closestCurve)
		return closestCurve;

	return model()->curve(0);
}


QPointF CurveView::closestPointOnCurve(const AbstractCurve& curve, const QPointF& pt)
{
	auto firstPos = curve.pos(firstPoint(curve));
	if (pt.x() < firstPos.x())
		return {pt.x(), firstPos.y()};

	auto lastPos = curve.pos(lastPoint(curve));
	if (pt.x() > lastPos.x())
		return {pt.x(), lastPos.y()};

	int samples = 32;

	QPointF closestPoint;
	qreal closestDist = std::numeric_limits<qreal>::max();
	for (int i = 0; i < samples; i++)
	{
		auto t = (qreal) i / (qreal) (samples - 1);
		auto x = firstPos.x() + (lastPos.x() - firstPos.x()) * t;
		auto p = QPointF(x, curve.evaluate(x));
		auto dist = length(p - pt);
		if (dist < closestDist)
		{
			closestPoint = p;
			closestDist = dist;
		}
	}

	return closestPoint;
}


int CurveView::firstPoint(const AbstractCurve& curve)
{
	int pcount = curve.pointCount();
	assert(pcount > 0);
	qreal limit = std::numeric_limits<qreal>::max();
	int idx = -1;
	for (int i = 0; i < pcount; i++)
	{
		auto x = curve.pos(i).x();
		if (x < limit)
		{
			idx = i;
			limit = x;
		}
	}
	assert(idx >= 0);
	return idx;
}


int CurveView::lastPoint(const AbstractCurve& curve)
{
	int pcount = curve.pointCount();
	assert(pcount > 0);
	qreal limit = -std::numeric_limits<qreal>::max();
	int idx = -1;
	for (int i = 0; i < pcount; i++)
	{
		auto x = curve.pos(i).x();
		if (x > limit)
		{
			idx = i;
			limit = x;
		}
	}
	return idx;
}


void CurveView::onCustomContextMenuRequested(const QPoint& pos)
{
	QMenu menu;

	auto selectedPoints = filterT<PointHandleItem>(scene()->selectedItems());
	auto selectedTangents = filterT<TangentHandleItem>(scene()->selectedItems());
	if (!selectedPoints.isEmpty())
	{
		menu.addSection("Interpolation");
		menu.addAction(&mInterpBezierAction);
		menu.addAction(&mInterpLinearAction);
		menu.addAction(&mInterpSteppedAction);
	}

	if (!selectedTangents.isEmpty() || !selectedPoints.isEmpty())
	{
		menu.addSection("Tangents");
		menu.addAction(&mSetTangentsAlignedAction);
		menu.addAction(&mSetTangentsBrokenAction);
		menu.addAction(&mFlattenTangentsAction);
	}

	if (!selectedPoints.isEmpty())
	{
		menu.addSection("Actions");
		menu.addAction(&mDeleteAction);
	}

	if (!menu.actions().isEmpty())
		menu.exec(mapToGlobal(pos));
}

void CurveView::setSelectedPointInterps(AbstractCurve::InterpType interp)
{
	for (auto pt :pointsFromSelection())
	{
		auto& segitem = pt->curveSegmentItem();
		int idx = segitem.index();
		auto& curve = segitem.curveItem().curve();
		curve.setInterpolation(idx, interp);
	}
}


void CurveView::setSelectedTangentsAligned(bool aligned)
{
	for (PointHandleItem* pt :pointsFromSelection())
	{
		int idx = pt->curveSegmentItem().index();
		auto& curve = pt->curveSegmentItem().curveItem().curve();
		bool wasAligned = curve.tangentsAligned(idx);
		curve.setTangentsAligned(idx, aligned);
		if (!wasAligned && aligned)
			alignTangents(curve, idx, true);
	}
}

void CurveView::setSelectedTangentsFlat()
{
	for (auto pt : pointsFromSelection())
	{
		int idx = pt->curveSegmentItem().index();
		auto& curve = pt->curveSegmentItem().curveItem().curve();
		auto inTan = curve.inTangent(idx);
		auto outTan = curve.outTangent(idx);
		curve.moveTangents({{idx, {inTan.x(), 0}}}, {{idx, {outTan.x(), 0}}}, true);
	}
}


void CurveView::alignTangents(AbstractCurve& curve, int idx, bool finished)
{
	const auto inTanPos = curve.inTangent(idx);
	const auto outTanPos = curve.outTangent(idx);
	qreal inMag = length(inTanPos);
	qreal outMag = length(outTanPos);
	const auto inVec = normalize(inTanPos);
	const auto outVec = normalize(outTanPos);

	QPointF antInPos = -outVec * inMag;
	QPointF antOutPos = -inVec * outMag;

	auto newInTanPos = lerpPoint(inTanPos, antInPos, 0.5);
	auto newOutTanPos = lerpPoint(outTanPos, antOutPos, 0.5);

	QMap<int, QPointF> inTangents = {{idx, newInTanPos}};
	QMap<int, QPointF> outTangents = {{idx, newOutTanPos}};
	curve.moveTangents(inTangents, outTangents, finished);
}


const QList<PointHandleItem*> CurveView::pointsFromSelection()
{
	QList<PointHandleItem*> points;
	for (auto item : scene()->selectedItems())
	{
		// If we have a point, include and on to the next
		auto point = dynamic_cast<PointHandleItem*>(item);
		if (point && !points.contains(point))
		{
			points << point;
			continue;
		}

		// No point, get point from selected tangent
		auto tan = dynamic_cast<TangentHandleItem*>(item);
		if (tan)
		{
			auto pt = &tan->pointHandle();
			if (!points.contains(pt))
				points << pt;
		}
	}
	return points;
}

void minSeparation(qreal& minVal, qreal& maxVal, qreal minSeparation)
{
	qreal dif = qAbs(minVal - maxVal);
	if (dif < minSeparation)
	{
		qreal expand = (minSeparation - dif) / 2;
		minVal -= expand;
		maxVal += expand;
	}
}

const QRectF CurveView::frameItemsBoundsSelected() const
{
	QList<QGraphicsItem*> handles = frameableItems(scene()->selectedItems());
	if (handles.isEmpty())
		return frameItemsBounds();

	return handleItemBounds(handles);
}

const QList<QGraphicsItem*> CurveView::frameableItems(const QList<QGraphicsItem*>& items) const
{
	QList<QGraphicsItem*> handles;
	for (auto item : scene()->selectedItems())
	{
		auto handle = dynamic_cast<HandleItem*>(item);
		if (handle)
		{
			auto& seg = handle->curveSegmentItem();
			if (!handles.contains(&seg.pointHandle()))
				handles << &seg.pointHandle();
			if (seg.inTanHandle().isVisible() && !handles.contains(&seg.inTanHandle()))
				handles << &seg.inTanHandle();
			if (seg.outTanHandle().isVisible() && !handles.contains(&seg.outTanHandle()))
				handles << &seg.outTanHandle();
		}
	}
	return handles;
}


const QRectF CurveView::frameItemsBounds() const
{
	QList<QGraphicsItem*> handles = frameableItems(scene()->items());
	if (handles.isEmpty())
		return {0, 0, 1, 1};

	return handleItemBounds(handles);
}

const QRectF CurveView::handleItemBounds(const QList<QGraphicsItem*>& handles) const
{
	const qreal minSize = 0.3;

	qreal left = std::numeric_limits<qreal>::infinity();
	qreal top = -std::numeric_limits<qreal>::infinity();
	qreal right = -std::numeric_limits<qreal>::infinity();
	qreal bottom = std::numeric_limits<qreal>::infinity();

	for (const auto h : handles)
	{
		left = qMin(left, h->pos().x());
		top = qMax(top, h->pos().y());
		right = qMax(right, h->pos().x());
		bottom = qMin(bottom, h->pos().y());
	}

	minSeparation(left, right, minSize);
	minSeparation(bottom, top, minSize);
	QRectF r;
	r.setLeft(left);
	r.setTop(bottom);
	r.setRight(right);
	r.setBottom(top);
	return r;
}


void CurveView::drawBackground(QPainter* painter, const QRectF& rect)
{
	GridView::drawBackground(painter, rect);

	if (mModel == nullptr)
		return;

	auto viewRect = mapToScene(viewport()->rect()).boundingRect();
	painter->setPen(QPen(Qt::gray, 0, Qt::DashLine));
	qreal viewStep = 3;
	qreal sceneStep = viewStep / getScale(transform()).width();
	for (int i = 0; i < model()->curveCount(); i++)
		drawCurveExtrapolation(painter, rect, viewRect, *model()->curve(i), sceneStep);
}


void CurveView::drawCurveExtrapolation(QPainter* painter, const QRectF& dirtyRect, const QRectF& viewRect,
									   const AbstractCurve& curve,
									   qreal step)
{
	qreal xmin = viewRect.left();
	qreal xmax = viewRect.right() + step;

	int pointCount = curve.pointCount();

	QPainterPath path;

	if (pointCount == 0)
	{
		path.moveTo(xmin, 0);
		path.lineTo(xmax, 0);
	}
	else if (pointCount == 1)
	{
		auto y = curve.pos(0).y();
		path.moveTo(xmin, y);
		path.lineTo(xmax, y);
	}
	else
	{
		QPointF firstPos(std::numeric_limits<qreal>::max(), 0);
		QPointF lastPos(-std::numeric_limits<qreal>::max(), 0);
		for (int i = 0; i < pointCount; i++)
		{
			auto pos = curve.pos(i);
			if (pos.x() < firstPos.x())
				firstPos = pos;
			if (pos.x() > lastPos.x())
				lastPos = pos;
		}

		path.moveTo(xmin, firstPos.y());
		path.lineTo(firstPos);
		path.moveTo(lastPos);
		path.lineTo(xmax, lastPos.y());
	}

	painter->drawPath(path);
}

void CurveView::selectCurves(const QList<AbstractCurve*>& curves)
{
	QList<QGraphicsItem*> handles;
	for (auto curve : curves)
	{
		auto item = curveItem(*curve);
		for (auto seg : item->segments())
		{
			handles << &seg->pointHandle();
		}
	}
	setSelection(handles);
}

QMap<AbstractCurve*, QList<int>> CurveView::selectedPoints()
{
	QMap<AbstractCurve*, QList<int>> curvePoints;

	for (auto sel : pointsFromSelection())
	{
		auto curve = &sel->curveSegmentItem().curveItem().curve();
		if (!curvePoints.contains(curve))
			curvePoints[curve] = QList<int>();

		curvePoints[curve].append(sel->curveSegmentItem().index());
	}
	return curvePoints;
}

void CurveView::setSelectedPointTimes(qreal t)
{
	QMap<AbstractCurve*, QMap<int, QPointF>> values;
	for (auto sel : pointsFromSelection())
	{
		auto& segItem = sel->curveSegmentItem();
		auto curve = &segItem.curveItem().curve();
		int idx = segItem.index();

		if (!values.contains(curve))
			values[curve] = QMap<int, QPointF>();

		auto& pt = curve->pos(idx);

		values[curve][sel->curveSegmentItem().index()] = {t, pt.y()};
	}
	mModel->movePoints(values);
}

void CurveView::setSelectedPointValues(qreal v)
{
	QMap<AbstractCurve*, QMap<int, QPointF>> values;
	for (auto sel : pointsFromSelection())
	{
		auto& segItem = sel->curveSegmentItem();
		auto curve = &segItem.curveItem().curve();
		int idx = segItem.index();

		if (!values.contains(curve))
			values[curve] = QMap<int, QPointF>();

		auto& pt = curve->pos(idx);

		values[curve][sel->curveSegmentItem().index()] = {pt.x(), v};
	}
	mModel->movePoints(values);
}

CurveItem* CurveView::curveItem(const AbstractCurve& curve)
{
	for (CurveItem* curveItem : mCurveItems)
		if (&curveItem->curve() == &curve)
			return curveItem;
	return nullptr;
}

void CurveView::commitPointEditChanges(bool finished)
{
	for (auto& key : mPointEditMap.keys())
	{
		CurveItem* curveItem = key;
		curveItem->curve().movePoints(mPointEditMap[key], finished);
	}

	for (CurveItem* curveItem : mTangentEditMap.keys())
	{
		auto tanPosList = mTangentEditMap[curveItem];
		curveItem->curve().moveTangents(tanPosList[0], tanPosList[1], finished);
	}
}

QList<QAction*> CurveView::interpActions()
{
	QList<QAction*> actions;
	actions << &mInterpLinearAction;
	actions << &mInterpSteppedAction;
	actions << &mInterpBezierAction;
	return actions;
}

QList<QAction*> CurveView::tangentActions()
{
	QList<QAction*> actions;
	actions << &mSetTangentsAlignedAction;
	actions << &mSetTangentsBrokenAction;
	actions << &mFlattenTangentsAction;
	return actions;
}

QList<QAction*> CurveView::auxiliaryActions()
{
	QList<QAction*> actions;
	actions << &mFrameViewAction;
	return actions;
}

CurveEditor::CurveEditor(QWidget* parent) : QWidget(parent)
{
	setLayout(&mLayout);
	mLayout.setSpacing(0);
	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.addWidget(&mToolbar);
	mLayout.addWidget(&mCurveView);

	mToolbar.setLayout(&mToolbarLayout);
	mToolbarLayout.setContentsMargins(2, 2, 2, 2);
	mToolbarLayout.setSpacing(2);

	mToolbarLayout.addWidget(&mTimeSpinbox);
	mTimeSpinbox.setEnabled(false);

	mToolbarLayout.addWidget(&mValueSpinbox);
	mValueSpinbox.setEnabled(false);

	mToolbarLayout.addWidget(new Separator(Qt::Vertical));

	for (const auto action : mCurveView.interpActions())
	{
		auto btAction = new QToolButton();
		btAction->setDefaultAction(action);
		mToolbarLayout.addWidget(btAction);
		action->setEnabled(false);
	}

	mToolbarLayout.addWidget(new Separator(Qt::Vertical));

	for (const auto action : mCurveView.tangentActions())
	{
		auto btAction = new QToolButton();
		btAction->setDefaultAction(action);
		mToolbarLayout.addWidget(btAction);
		action->setEnabled(false);
	}

	mToolbarLayout.addWidget(new Separator(Qt::Vertical));

	for (const auto action : mCurveView.auxiliaryActions())
	{
		auto btAction = new QToolButton();
		btAction->setDefaultAction(action);
		mToolbarLayout.addWidget(btAction);
//		action->setEnabled(true);
	}

	connect(&mCurveView, &CurveView::selectionChanged, this, &CurveEditor::onSelectionChanged);
	connect(&mTimeSpinbox, &FloatLineEdit::valueChanged, this, &CurveEditor::onTimeChanged);
	connect(&mValueSpinbox, &FloatLineEdit::valueChanged, this, &CurveEditor::onValueChanged);
}

void CurveEditor::onTimeChanged(qreal t)
{
	mCurveView.setSelectedPointTimes(t);
}

void CurveEditor::onValueChanged(qreal v)
{
	mCurveView.setSelectedPointValues(v);
}


CurveEditor::~CurveEditor()
{
	disconnect(&mCurveView, &CurveView::selectionChanged, this, &CurveEditor::onSelectionChanged);
	disconnect(&mTimeSpinbox, &FloatLineEdit::valueChanged, this, &CurveEditor::onTimeChanged);
	disconnect(&mValueSpinbox, &FloatLineEdit::valueChanged, this, &CurveEditor::onValueChanged);
}


void CurveEditor::setModel(AbstractCurveModel* model)
{
	if (mCurveModel)
	{
	}

	mCurveModel = model;
	mCurveView.setModel(mCurveModel);

	if (mCurveModel)
	{
		for (int i = 0, len = mCurveModel->curveCount(); i < len; i++)
		{
			connect(model->curve(i), &AbstractCurve::pointsChanged, [this](auto indices, bool finished)
			{
				onPointsChanged();
			});
		}
	}
}


void CurveEditor::onSelectionChanged(QMap<AbstractCurve*, QList<int>> points)
{
	bool hasPointSelection = !points.isEmpty();

	mTimeSpinbox.setEnabled(hasPointSelection);
	mValueSpinbox.setEnabled(hasPointSelection);
	for (const auto action : mCurveView.interpActions())
		action->setEnabled(hasPointSelection);
	for (const auto action : mCurveView.tangentActions())
		action->setEnabled(hasPointSelection);

	mTimeSpinbox.blockSignals(true);
	mValueSpinbox.blockSignals(true);

	mTimeSpinbox.setIndeterminate(true);
	mValueSpinbox.setIndeterminate(true);

	if (hasPointSelection)
	{
		bool singlePoint = points.size() == 1 && points.first().size() == 1;
		if (singlePoint)
		{
			auto curve = points.firstKey();
			int idx = points.first()[0];
			auto pt = curve->pos(idx);

			mTimeSpinbox.setValue(pt.x());
			mValueSpinbox.setValue(pt.y());
			mTimeSpinbox.setIndeterminate(false);
			mValueSpinbox.setIndeterminate(false);
		}
	}
	else
	{
		mTimeSpinbox.setValue(0);
		mValueSpinbox.setValue(0);
	}

	mTimeSpinbox.blockSignals(false);
	mValueSpinbox.blockSignals(false);
}

void CurveEditor::onPointsChanged()
{
	onSelectionChanged(mCurveView.selectedPoints());
}


