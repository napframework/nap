#include "utils/include.h"
#include <napqt/curveeditor/standardcurve.h>

using namespace nap::qt;

TEST_CASE("CurveEditor")
{
	RUN_Q_APPLICATION

	SECTION("Add/Remove Curve")
	{
		StandardCurveModel model;
		REQUIRE(model.curveCount() == 0);

		StandardCurve* curve = model.addCurve();
		REQUIRE(curve != nullptr);
		REQUIRE(model.curveCount() == 1);

		model.removeCurve(0);
		REQUIRE(model.curveCount() == 0);

		curve = model.addCurve();
		REQUIRE(curve != nullptr);
		REQUIRE(model.curveCount() == 1);
	}

	SECTION("Add/Remove Points")
	{
		StandardCurve curve;
		REQUIRE(curve.pointCount() == 0);

		curve.addPoint(0.0, 0.0);
		REQUIRE(curve.pointCount() == 1);

		curve.addPoint(0.1, 0.1);
		REQUIRE(curve.pointCount() == 2);

		curve.addPoint(0.2, 0.2);
		REQUIRE(curve.pointCount() == 3);

		curve.removePoint(1);
		REQUIRE(curve.pointCount() == 2);

		curve.removePoint(1);
		REQUIRE(curve.pointCount() == 1);
	}
}