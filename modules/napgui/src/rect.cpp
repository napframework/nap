#include "rect.h"
#include "layoutcomponent.h"

RTTI_DEFINE_DATA(nap::Point)
RTTI_DEFINE_DATA(nap::Rect)
RTTI_DEFINE_DATA(nap::Margins)

namespace nap
{


	void Point::set(float x, float y)
	{
		mX = x;
		mY = y;
	}

	Point Point::operator+(const Point& other) const { return Point(mX + other.getX(), mY + other.getY()); }

	Point Point::operator-(const Point& other) const { return Point(mX - other.getX(), mY - other.getY()); }

	Point& Point::operator+=(const Point& other) { set(mX + other.getX(), mY + other.getY()); return *this; }
	Point& Point::operator-=(const Point& other) { set(mX - other.getX(), mY - other.getY()); return *this; }

	void Rect::set(float x, float y, float width, float height)
	{
		mX = x;
		mY = y;
		mWidth = width;
		mHeight = height;
	}

	Rect::Rect(float x, float y, float width, float height)
	{
		setX(x);
		setY(y);
		setWidth(width);
		setHeight(height);
	}

	void Rect::setPos(float x, float y)
	{
		mX = x;
		mY = y;
	}

	void Rect::setSize(float w, float h)
	{
		mWidth = w;
		mHeight = h;
	}

    void Rect::setWidthAndRatio(float w, float ratio) {
        mWidth = w;
        mHeight = mWidth / ratio;
    }

    void Rect::setHeightAndRatio(float h, float ratio) {
        mHeight = h;
        mWidth = mHeight * ratio;
    }

    Margins::Margins(float left, float top, float right, float bottom)
		: mLeft(left), mTop(top), mRight(right), mBottom(bottom)
	{
	}
}