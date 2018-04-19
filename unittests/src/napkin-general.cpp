#include "utils/catch.hpp"

#include <appcontext.h>
#include <commands.h>
#include <generic/naputils.h>
#include <QCoreApplication>

#include "utils/testclasses.h"
#include "utils/testutils.h"

using namespace napkin;


TEST_CASE("File Extensions", "napkin-general")
{
	int argc = 0;
	char* argv[] = {"app"};
	QApplication app(argc, const_cast<char**>(argv));


	ResourceFactory fact = AppContext::get().getResourceFactory();
	{
		QStringList imageExtensions;
		for (const auto& e : fact.getImageExtensions())
			imageExtensions << e;
		REQUIRE(imageExtensions.size() > 0);

		auto imageExtString = "Image Extensions: " + imageExtensions.join(", ");
		REQUIRE(!imageExtString.isEmpty());

		nap::Logger::debug(imageExtString.toStdString());
	}
	{
		QStringList videoExtensions;
		for (const auto& e : fact.getVideoExtensions())
			videoExtensions << e;
		REQUIRE(videoExtensions.size() > 0);

		auto videoExtString = "Video Extensions: " + videoExtensions.join(", ");
		REQUIRE(!videoExtString.isEmpty());

		nap::Logger::debug(videoExtString.toStdString());
	}
}

TEST_CASE("Resource Management", "napkin-general")
{
	int argc = 0;
	char* argv[] = {"app"};
	QApplication app(argc, const_cast<char**>(argv));

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
	REQUIRE(directoryContains(resourcedir, absShaderPath));
	REQUIRE(directoryContains(resourcedir, absJsonFilePath));
	REQUIRE(directoryContains(resourcedir + "/shaders", absShaderPath));
	// or not
	REQUIRE(!directoryContains(absShaderPath, resourcedir));

}

