#include "utils/include.h"

#include <appcontext.h>
#include <commands.h>
#include <naputils.h>

using namespace napkin;

TEST_CASE("Resource Management", "napkin-general")
{
	/*
	napkin::AppContext::create();
    RUN_Q_APPLICATION

	// Assume this test file's directory as the base path
	QString jsonFile = "objects.json";
	QString shaderFile = "shaders/debug.frag";
	QString dataDir = ".";

	// Just set the filename for reference purposes
	AppContext::get().getDocument()->setFilename(getResource(jsonFile));

	// Ensure shader file exists
	auto absJsonFilePath = QFileInfo(getResource(jsonFile)).absoluteFilePath();
	REQUIRE(QFileInfo::exists(absJsonFilePath));

	// Ensure shader file exists
	auto absShaderFilePath = QFileInfo(getResource(shaderFile)).absoluteFilePath();
	REQUIRE(QFileInfo::exists(absShaderFilePath));

	// Ensure the resource directory exists
	auto resourcedir = QFileInfo(getResource("")).absoluteFilePath();
	REQUIRE(QFileInfo::exists(resourcedir));

	// Test our local resource function with napkin's
	auto basedir = QFileInfo(getResourceReferencePath()).absoluteFilePath();
	REQUIRE(basedir.toStdString() == resourcedir.toStdString());

	// Check relative path
	auto relJsonPath = getRelativeResourcePath(absJsonFilePath);
	REQUIRE(relJsonPath.toStdString() == jsonFile.toStdString());


	// Check relative path
	auto relShaderPath = getRelativeResourcePath(absShaderFilePath);
	REQUIRE(relShaderPath.toStdString() == shaderFile.toStdString());

	// Check absolute path
	auto absShaderPath = getAbsoluteResourcePath(relShaderPath);
	REQUIRE(absShaderPath.toStdString() == absShaderFilePath.toStdString());

	// Check if dir contains path
	REQUIRE(nap::qt::directoryContains(resourcedir, absShaderPath));
	REQUIRE(nap::qt::directoryContains(resourcedir, absJsonFilePath));
	REQUIRE(nap::qt::directoryContains(resourcedir + "/shaders", absShaderPath));
	// or not
	REQUIRE(!nap::qt::directoryContains(absShaderPath, resourcedir));
	
	napkin::AppContext::destroy();
	*/
}

