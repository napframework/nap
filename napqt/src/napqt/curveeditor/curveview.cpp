#include "curveview.h"

#include <cassert>

#include <QAction>
#include <QKeySequence>
#include <QList>
#include <QMap>
#include <QMenu>
#include <QtDebug>
#include <QtGui>
#include <QVector>

#include <napqt/qtutils.h>
#include <napqt/timeline/eventitem.h>

#include "curvemath.h"

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
void TangentHandleItem::updateRect()
{
	if (curveSegmentItem().curveItem().curve().data(curveSegmentItem().index(), datarole::TAN_ALIGNED).toBool())
	{
		mPath = QPainterPath();
		auto ext = isSelected() ? mExtentSelectd : mExtent;
		mRect.setCoords(-ext, -ext, ext * 2, ext * 2);
		mPath.addEllipse(mRect);
	}
	else
	{
		mPath = QPainterPath();
		auto ext = isSelected() ? mExtentSelectd : mExtent;
		mRect.setCoords(-ext, -ext, ext * 2, ext * 2);
		mPath.addRect(mRect);
	}
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
	bool bb = !isFirstPoint();
	if (bb)
	{
		const auto& prevInterp = curveItem().curve().data(curveItem().prevSegIndex(index()),
														  datarole::INTERP).value<AbstractCurve::InterpType>();
		bb = bb && prevInterp == AbstractCurve::InterpType::Bezier;
	}
	setInTanVisible(bb && b);

	const auto& interp = curveItem().curve().data(index(), datarole::INTERP).value<AbstractCurve::InterpType>();
	setOutTanVisible(b && !isLastPoint() && interp == AbstractCurve::InterpType::Bezier);
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
	mInTanHandle.updateRect();
	mOutTanHandle.setPos(outTanPos);
	mOutTanHandle.updateRect();

	mInTanLine.setFromTo(pos, inTanPos);
	mOutTanLine.setFromTo(pos, outTanPos);

	if (mPointHandle.isSelected())
		setTangentsVisible(true);

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
			qreal v;
			switch (interp) {
				case AbstractCurve::InterpType::Bezier:
					v = evalCurveSegmentBezier({a, b, c, d}, x);
					break;
				case AbstractCurve::InterpType::Linear:
					v = evalCurveSegmentLinear({a, b, c, d}, x);
					break;
				case AbstractCurve::InterpType::Stepped:
					v = evalCurveSegmentStepped({a, b, c, d}, x);
					break;
				default:
					assert(false);
			}
			mDebugPath.lineTo(x, v);
		}
		mDebugPath.lineTo(d.x(), d.y());

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
	setContextMenuPolicy(Qt::NoContextMenu);

	initActions();
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
		else if (ctrlHeld)
		{
			auto scenePos = mapToScene(event->pos());
			auto curveItem = closestCurveItem(event->pos());
			if (curveItem)
				curveItem->curve().addPoint(scenePos.x(), scenePos.y());
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
		bool aligned = curveItem->curve().data(idx, datarole::TAN_ALIGNED).toBool();
		if (aligned)
		{
			auto otherPos = alignedOppositeTangentVector(*tangent);

			if (otherTan.isInTangent())
				positions[0][idx] = otherPos - otherTan.pointHandle().pos();
			else
				positions[1][idx] = otherPos - otherTan.pointHandle().pos();
		}
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

void CurveView::initActions()
{
	mDeleteAction.setText("Delete");
	mDeleteAction.setShortcut(QKeySequence::Delete);
	mDeleteAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(&mDeleteAction, &QAction::triggered, this, &CurveView::deleteSelectedItems);
	addAction(&mDeleteAction);

	mInterpBezierAction.setText("Bezier");
	connect(&mInterpBezierAction, &QAction::triggered,
			[this]() { setSelectedPointInterps(AbstractCurve::InterpType::Bezier); });

	mInterpLinearAction.setText("Linear");
	connect(&mInterpLinearAction, &QAction::triggered,
			[this]() { setSelectedPointInterps(AbstractCurve::InterpType::Linear); });

	mInterpSteppedAction.setText("Stepped");
	connect(&mInterpSteppedAction, &QAction::triggered,
			[this]() { setSelectedPointInterps(AbstractCurve::InterpType::Stepped); });

	mToggleAlignedAction.setText("Aligned");
	mToggleAlignedAction.setCheckable(true);
	connect(&mToggleAlignedAction, &QAction::triggered,
			[this]() { setSelectedTangentsAligned(mToggleAlignedAction.isChecked()); });
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
			toDelete[curveIndex] = QList < int > ();

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

CurveItem* CurveView::closestCurveItem(const QPointF& pos)
{
	for (auto item : items(pos.x(), pos.y()))
	{
		auto segment = dynamic_cast<CurveSegmentItem*>(item);
		if (segment)
			return &segment->curveItem();
	}
	return nullptr;
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
		menu.addAction(&mToggleAlignedAction);

		int alignedCount = 0;
		int nonAlignedCount = 0;
		for (PointHandleItem* item : pointsFromSelection())
		{
			bool aligned = item->curveSegmentItem().curveItem().curve().data(item->curveSegmentItem().index(),
																			 datarole::TAN_ALIGNED).toBool();
			if (aligned)
				alignedCount++;
			else
				nonAlignedCount++;
		}
		mToggleAlignedAction.setChecked(alignedCount > nonAlignedCount);
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
	for (PointHandleItem* pt :pointsFromSelection())
	{
		int idx = pt->curveSegmentItem().index();
		auto& curve = pt->curveSegmentItem().curveItem().curve();
		auto oldInterp = curve.data(idx, datarole::TAN_ALIGNED).value<AbstractCurve::InterpType>();
		curve.setData(idx, datarole::INTERP, QVariant::fromValue(interp));
//		if (oldInterp != interp)
//		{
//		}
	}
}


void CurveView::setSelectedTangentsAligned(bool aligned)
{
	for (PointHandleItem* pt :pointsFromSelection())
	{
		int idx = pt->curveSegmentItem().index();
		auto& curve = pt->curveSegmentItem().curveItem().curve();
		bool wasAligned = curve.data(idx, datarole::TAN_ALIGNED).toBool();
		curve.setData(idx, datarole::TAN_ALIGNED, aligned);
		if (!wasAligned && aligned)
			alignTangents(curve, idx);
	}
}

void CurveView::alignTangents(AbstractCurve& curve, int idx)
{
	QPointF inTanPos = curve.data(idx, datarole::IN_TAN).toPointF();
	QPointF outTanPos = curve.data(idx, datarole::OUT_TAN).toPointF();
	qreal inMag = length(inTanPos);
	qreal outMag = length(outTanPos);
	QPointF inVec = normalize(inTanPos);
	QPointF outVec = normalize(outTanPos);

	QPointF antInPos = -outVec * inMag;
	QPointF antOutPos = -inVec * outMag;

	auto newInTanPos = lerpPoint(inTanPos, antInPos, 0.5);
	auto newOutTanPos = lerpPoint(outTanPos, antOutPos, 0.5);

	curve.setData(idx, datarole::IN_TAN, newInTanPos);
	curve.setData(idx, datarole::OUT_TAN, newOutTanPos);
}


const QList<PointHandleItem*> CurveView::pointsFromSelection()
{
	QList < PointHandleItem * > points;
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
