#include "curveview.h"
#include "curvemath.h"

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

HandleItem::HandleItem() : QObject(), QGraphicsItem()
{
	setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	setFlag(QGraphicsItem::ItemIsMovable, true);
	setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);
	setSelected(true);

	mRect.setCoords(-mExtent, -mExtent, mExtent * 2, mExtent * 2);
	mHitRect.setCoords(-mHitExtent, -mHitExtent, mHitExtent * 2, mHitExtent * 2);
	mPath.addRect(mRect);
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
		moved(this);
	} else if (change == QGraphicsItem::ItemSelectedHasChanged)
	{
		selected(this);
	}
	return res;
}

void HandleItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setPen(mPen);
	painter->setBrush(isSelected() ? mBrushSelected : mBrush);
	painter->drawPath(mPath);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PointHandleItem
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PointHandleItem::PointHandleItem()
{
	mPen = QPen(Qt::NoPen);
	mBrush = QBrush("#840");
	mBrushSelected = QBrush("#000");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TangentHandleItem
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TangentHandleItem::TangentHandleItem()
{
	mPen = QPen(Qt::NoPen);
	mBrush = QBrush("#080");
	mBrushSelected = QBrush("#0F0");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TangentLineItem
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LineItem::LineItem()
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

CurveSegmentItem::CurveSegmentItem()
{
	setZValue(-200);
	setFlag(QGraphicsItem::ItemIsSelectable, true);

	mPen = QPen(Qt::blue, 0);
	mPenDebug = QPen(Qt::gray, 0);
	mPenSelected = QPen(Qt::red, 0);

	mPointHandle.setParentItem(this);
	mInTanHandle.setParentItem(this);
	mOutTanHandle.setParentItem(this);
	mInTanLine.setParentItem(this);
	mOutTanLine.setParentItem(this);
}

void CurveSegmentItem::setPoints(const QPointF (& pts)[4])
{
	for (int i = 0; i < 4; i++)
		mPoints[i] = pts[i];
	refreshCurve();
}

void CurveSegmentItem::refreshCurve()
{
	mPath = QPainterPath();
	mPath.moveTo(mPoints[0]);

	mDebugPath = QPainterPath();

	qreal xmin = mPoints[0].x();
	qreal xmax = mPoints[3].x();
	qreal xd = xmax - xmin;

	for (int i = 0; i < mSampleCount; i++)
	{
		qreal t = (qreal) i / (qreal) mSampleCount;
		qreal x = xmin + xd + t;
		mPath.lineTo(x, evalCurveSegment(mPoints, x));
	}
	mPath.lineTo(mPoints[3]);
	parentItem()->update();

	mDebugPath.moveTo(mPoints[0]);
	mDebugPath.cubicTo(mPoints[1], mPoints[2], mPoints[3]);
}

QRectF CurveSegmentItem::boundingRect() const
{
	return mPath.boundingRect().united(mDebugPath.boundingRect());
}

void CurveSegmentItem::setFirstPoint(bool b)
{
	mInTanHandle.setVisible(!b);
	mInTanLine.setVisible(!b);
}

void CurveSegmentItem::setLastPoint(bool b)
{
	mOutTanHandle.setVisible(!b);
	mOutTanLine.setVisible(!b);
}

void CurveSegmentItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setPen(mPenDebug);
	painter->drawPath(mDebugPath);

	painter->setPen(isSelected() ? mPenSelected : mPen);
	painter->drawPath(mPath);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CurveItem
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


CurveItem::CurveItem(QGraphicsItem* parent, AbstractCurve& curve) : QObject(), QGraphicsPathItem(parent), mCurve(curve)
{
	connect(&curve, &AbstractCurve::pointsChanged, this, &CurveItem::onPointsChanged);
	connect(&curve, &AbstractCurve::pointsAdded, this, &CurveItem::onPointsAdded);
	connect(&curve, &AbstractCurve::pointsRemoved, this, &CurveItem::onPointsRemoved);

	QList<int> indices;
	for (int i=0, len=curve.pointCount(); i < len; i++)
		indices << i;
	onPointsAdded(indices);
}

void CurveItem::onPointsChanged(const QList<int> indices)
{
	setPointOrderDirty();
	for (int i : indices)
		updateSegmentFromPoint(i);
}

void CurveItem::updateSegmentFromPoint(int i)
{
	auto seg = mSegments[i];
	auto pos = mCurve.data(i, datarole::POS).toPointF();
	auto inTan = mCurve.data(i, datarole::IN_TAN).toPointF();
	auto outTan = mCurve.data(i, datarole::OUT_TAN).toPointF();

	seg->pointHandle().setPos(pos);
	seg->inTanHandle().setPos(pos + inTan);
	seg->outTanHandle().setPos(pos + outTan);

	bool isFirst = i == 0;
	bool isLast = i == mSegments.size() - 1;
	seg->setFirstPoint(isFirst);
	seg->setLastPoint(isLast);

	if (!isLast) {
		int nextIndex = sortedPoints()[i+1];
		auto nextPos = mCurve.data(nextIndex, datarole::POS).toPointF();
		auto nextTan = mCurve.data(nextIndex, datarole::IN_TAN).toPointF();

		QPointF pts[4];
		pts[0] = pos;
		pts[1] = pos + outTan;
		pts[2] = pos + nextTan;
		pts[3] = nextPos;
		seg->setPoints(pts);
	}
}

CurveSegmentItem* CurveItem::nextSegment(int i)
{
	if (i >= mSegments.size())
		return nullptr;

	auto pointMap = sortedPoints();

	return nullptr;
}

void CurveItem::onPointsAdded(const QList<int> indices)
{
	QList<CurveSegmentItem*> segments;
	for (int index : indices)
	{
		auto seg = new CurveSegmentItem();
		seg->setParentItem(this);
		mSegments.insert(index, seg);
		mSortedPoints.insert(index, mSortedPoints.size());
		segments << seg;
	}

	// Push changes from model too
	QList<int> idx;
	for (auto seg : segments)
		idx << segmentIndex(seg);

	setPointOrderDirty();
	onPointsChanged(idx);
}

void CurveItem::onPointsRemoved(const QList<int> indices)
{
	auto sortedIndices = reverseSort(indices);

	for (int index : sortedIndices)
	{
		mSortedPoints.removeAt(index);
		auto seg = mSegments.takeAt(index);
		delete seg;
	}
	setPointOrderDirty();
}

int CurveItem::segmentIndex(CurveSegmentItem* item) const
{
	for (int i = 0, len = mSegments.size(); i < len; i++)
		if (mSegments.at(i) == item)
			return i;
	return -1;
}

int CurveItem::sortedIndex(CurveSegmentItem* seg)
{
	int index = segmentIndex(seg);
	auto pointMap = sortedPoints();
	for (int i=0, len=pointMap.size(); i<len; i++)
	{
		int sortedIndex = pointMap.at(i);
		if (mSegments[sortedIndex] == seg)
			return index;
	}
	return -1;
}

void CurveItem::setPointOrderDirty()
{
	mPointOrderDirty = true;
}

const QList<int>& CurveItem::sortedPoints()
{
	if (!mPointOrderDirty)
		return mSortedPoints;

	mSortedPoints.clear();
	for (int i = 0, len = mCurve.pointCount(); i < len; i++)
		mSortedPoints.append(i);

	qSort(mSortedPoints.begin(), mSortedPoints.end(), [this](const QVariant& a, const QVariant& b)
	{
		bool ok;
		qreal timeA = mCurve.data(a.toInt(&ok), datarole::POS).toPointF().x();
		qreal timeB = mCurve.data(a.toInt(&ok), datarole::POS).toPointF().y();
		return timeA < timeB;
	});

	mPointOrderDirty = false;
	return mSortedPoints;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CurveScene
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CurveScene::CurveScene() : QGraphicsScene()
{
	setSceneRect(-DEFAULT_SCENE_EXTENT, -DEFAULT_SCENE_EXTENT,
				 DEFAULT_SCENE_EXTENT * 2, DEFAULT_SCENE_EXTENT * 2);
}

void CurveScene::setModel(AbstractCurveModel* model)
{
	if (mModel == model)
		return;

	if (mModel)
	{
		disconnect(mModel, &AbstractCurveModel::curvesAdded, this, &CurveScene::onCurvesAdded);
		disconnect(mModel, &AbstractCurveModel::curvesAdded, this, &CurveScene::onCurvesAdded);
	}

	clear();

	mModel = model;

	if (mModel)
	{
		connect(mModel, &AbstractCurveModel::curvesAdded, this, &CurveScene::onCurvesAdded);
		connect(mModel, &AbstractCurveModel::curvesAdded, this, &CurveScene::onCurvesAdded);
	}

	QList<int> indices;
	for (int i = 0, len = mModel->curveCount(); i < len; i++)
		indices << i;
	onCurvesAdded(indices);
}

void CurveScene::onCurvesAdded(QList<int> indices)
{
	for (int index : indices)
	{
		auto curve = mModel->curve(index);
		auto curveItem = new CurveItem(nullptr, *curve);
		addItem(curveItem);
		mCurveItems.insert(index, curveItem);
	}
}
void CurveScene::onCurvesRemoved(QList<int> indices)
{
	auto sortedIndexes = reverseSort(indices);
	for (int index : sortedIndexes)
	{
		auto curveItem = mCurveItems.takeAt(index);
		delete curveItem;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CurveView
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


CurveView::CurveView(QWidget* parent) : GridView(parent)
{
	setScene(&mCurveScene);
	setPanZoomMode(PanMode::Parallax, ZoomMode::IgnoreAspectRatio);
	setFramePanZoomMode(PanMode::Parallax, ZoomMode::IgnoreAspectRatio);
	frameView(QRectF(0, 0, 1, 1), QMargins(10, 10, 10, 10));
//	setGridIntervalDisplay(stFloatIntervalDisplay(), FloatIntervalDisplay());
}

