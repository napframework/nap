// Local Includes
#include "npolyline.h"

namespace opengl
{
	void PolyLine::draw()
	{
		DrawMode mode = mClosed ? DrawMode::LINE_LOOP : DrawMode::LINE_STRIP;
		mObject.setDrawMode(mode);
		mObject.draw();
	}
} // opengl