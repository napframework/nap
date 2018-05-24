#include "testutils.h"

QString getResource(const QString& filename)
{
	const QString resourceDir = "unit_tests_data";
	if (filename.isEmpty())
		return resourceDir;
	return resourceDir + "/" + filename;
}
