#pragma once

#include <nap.h>

namespace nap
{

	class Point
	{
		RTTI_ENABLE()
	public:
		Point() {}
		Point(float x, float y) : mX(x), mY(y) {}

		void set(float x, float y);
		void setX(float x) { mX = x; }
		void setY(float y) { mY = y; }

		float getX() const { return mX; }
		float getY() const { return mY; }

		Point operator+(const Point& other) const;
		Point operator-(const Point& other) const;
        Point& operator+=(const Point& other);
        Point& operator-=(const Point& other);
		bool operator == (const Point& other) const;

	private:
		float mX = 0;
		float mY = 0;
	};

	// General-purpose rectangle for layout
	class Rect
	{
		RTTI_ENABLE()

	public:
		Rect(){};
		Rect(float x, float y, float width, float height);

		void set(float x, float y, float width, float height);

		void setPos(float x, float y);

		void setSize(float w, float h);

        void setWidthAndRatio(float w, float ratio);
        void setHeightAndRatio(float h, float ratio);

		void setX(float x) { mX = x; }
		const float& getX() const { return mX; }

		void setY(float y) { mY = y; }
		const float& getY() const { return mY; }

		float getWidth() const { return mWidth; }
		void setWidth(float width) { mWidth = width; }

		float getHeight() const { return mHeight; }
		void setHeight(float height) { mHeight = height; }

		Point getTopLeft() const { return Point(getX(), getY()); }

		bool operator == (const Rect& other) const;

		Rect operator*(const float& scale) const;

	private:
		float mX = 0;
		float mY = 0;
		float mWidth = 0;
		float mHeight = 0;
	};

	class Margins
	{
		RTTI_ENABLE()
	public:
		Margins() : mLeft(0), mTop(0), mRight(0), mBottom(0) {}
		Margins(float left, float top, float right, float bottom);

		const float& getLeft() const { return mLeft; }
		const float& getTop() const { return mTop; }
		const float& getRight() const { return mRight; }
		const float& getBottom() const { return mBottom; }

		const Point getTopLeft() const { return Point(getTop(), getLeft()); }

		void setLeft(const float& left) { mLeft = left; }
		void setTop(const float& top) { mTop = top; }
		void setRight(const float& right) { mRight = right; }
		void setBottom(const float& bottom) { mBottom = bottom; }

		bool operator == (const Margins& other) const;

	private:
		float mLeft;
		float mTop;
		float mRight;
		float mBottom;
	};
}
RTTI_DECLARE_DATA(nap::Point)
RTTI_DECLARE_DATA(nap::Rect)
RTTI_DECLARE_DATA(nap::Margins)
