/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "curvemath.h"

void nap::qt::limitOverhang(qreal& x0, qreal& x1, qreal& x2, qreal& x3)
{
	x1 = qMin(x1, x3);
	x2 = qMax(x2, x0);
}

void nap::qt::limitOverhangQPoints(QPointF& pa, QPointF& pb, QPointF& pc, QPointF& pd)
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
