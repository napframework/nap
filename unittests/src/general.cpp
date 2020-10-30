#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file

#include "utils/catch.hpp"
#include <audio/utility/safeptr.h>
#include <utility/fileutils.h>

TEST_CASE("File path transformations", "[fileutils]")
{
	// Extensions
	REQUIRE(nap::utility::getFileExtension("/home/test/one.ext") != ".ext");
	REQUIRE(nap::utility::getFileExtension("/home/test/one.ext") == "ext");
	REQUIRE(nap::utility::getFileExtension("poekie.poes.png") == "png");
	REQUIRE(nap::utility::getFileExtension("tommy.toedel.") == "");
	REQUIRE(nap::utility::getFileExtension("file-name.longextension") == "longextension");

//	REQUIRE(utility::getFileDir("/home/someone//filename.ext") == "/home/someone");
	REQUIRE(nap::utility::getFileDir("/home/someone/filename.ext") == "/home/someone");

	REQUIRE(nap::utility::getFileName("/home/someone/filename.ext") == "filename.ext");
	REQUIRE(nap::utility::stripFileExtension("/home/someone/filename.ext") == "/home/someone/filename");
	REQUIRE(nap::utility::getFileNameWithoutExtension("/home/someone/filename.ext") == "filename");

	REQUIRE(nap::utility::appendFileExtension("/cash/cow", "png") == "/cash/cow.png");
	REQUIRE(nap::utility::hasExtension("foo.bar", "bar"));
	REQUIRE(!nap::utility::hasExtension("foo.bar", "ar"));
	REQUIRE(nap::utility::hasExtension("foo.foo.bar", "bar"));
	REQUIRE(!nap::utility::hasExtension("foo.foo.bar", "foo.bar"));

	// TODO: Make more of this sweet stuff
}


TEST_CASE("String utilities", "[stringutils]")
{
	SECTION("splitString")
	{
		{
			auto split = nap::utility::splitString("souffleur", '.');
			REQUIRE(split.size() == 1);
			REQUIRE(split[0] == "souffleur");
		}
		{
			auto split = nap::utility::splitString("one.two.three", '.');
			REQUIRE(split.size() == 3);
			REQUIRE(split[0] == "one");
			REQUIRE(split[1] == "two");
			REQUIRE(split[2] == "three");
		}
		{
			auto split = nap::utility::splitString("one/", '/');
			REQUIRE(split.size() == 1);
			REQUIRE(split[0] == "one");
		}
		{
			auto split = nap::utility::splitString("/", '/');
			REQUIRE(split.size() == 1);
			REQUIRE(split[0] == "");
		}
		{
			auto split = nap::utility::splitString("double//slash", '/');
			REQUIRE(split.size() == 3);
			REQUIRE(split[0] == "double");
			REQUIRE(split[1] == "");
			REQUIRE(split[2] == "slash");
		}
	}
	SECTION("joinString")
	{
		std::vector<std::string> vec;
		REQUIRE(nap::utility::joinString(vec, "?") == "");

		vec = {"one"};
		REQUIRE(nap::utility::joinString(vec, "!?!?") == "one");

		vec = {"the", "highway", "to", "hell"};
		REQUIRE(nap::utility::joinString(vec, "/") == "the/highway/to/hell");
		REQUIRE(nap::utility::joinString(vec, ", ") == "the, highway, to, hell");
		REQUIRE(nap::utility::joinString(vec, "") == "thehighwaytohell");
	}

}

/*
TEST_CASE("Template Replacement", "[stringutils]")
{
	{
		std::string a = "You {REP} pass!";
		std::unordered_map<std::string, std::string> replacements = {
			{"REP", "can not"},
		};
		auto result = nap::utility::namedFormat(a, replacements);
		REQUIRE(result == "You can not pass!");
	}

	{
		std::string a = "/the/{PATH}/to/{PARM}/{SOMETHING}/{PATH}";
		std::unordered_map<std::string, std::string> replacements = {
			{"PATH", "road"},
			{"PARM", "success"},
		};
		auto result = nap::utility::namedFormat(a, replacements);
		REQUIRE(result == "/the/road/to/success/{SOMETHING}/road");
	}
}
*/

TEST_CASE("DateTime Utilities", "[datetime]")
{
    // TODO In July 2020 this test is broken for machines running on a timezone which have DST
    //      at the used datetime, at least when running outside DST (seen on Linux)

	auto currenttime = nap::getCurrentTime();
	auto flc1_launch_str = "2006-03-24 22:30:01.123";
	auto flc1_launch = nap::createTimestamp(2006, 03, 24, 22, 30, 01, 123);

	nap::DateTime flc1_launch_date(flc1_launch);
	REQUIRE(flc1_launch_date.getYear() == 2006);
	REQUIRE(flc1_launch_date.getMonth() == nap::EMonth::March);
	REQUIRE(flc1_launch_date.getDayInTheMonth() == 24);
	REQUIRE(flc1_launch_date.getHour() == 22);
	REQUIRE(flc1_launch_date.getMinute() == 30);
	REQUIRE(flc1_launch_date.getSecond() == 01);
	REQUIRE(flc1_launch_date.getMilliSecond() == 123);

	REQUIRE(nap::timeFormat(flc1_launch) == flc1_launch_str);
}

TEST_CASE("Safe pointers", "[safepointer]")
{
	using namespace nap::audio;

	class TestBase
	{
	public:
		virtual ~TestBase() = default;
		virtual int getX() = 0;
	};

	// Test object that increments a counter when it's constructed and decrements it on destruction
	class Test : public TestBase
	{
	public:
		Test(int x, int& destructed) : mX(x), mCounter(destructed) { mCounter++; }
		~Test() { mCounter--; }
		int getX() override { return mX; }
		int mX = 0;
		int& mCounter;
	};

	int counter = 0; // The counter to count the number of existing objects
	DeletionQueue deletionQueue; // The DeletionQueue used
	SafePtr<Test> safePtr = nullptr;
	SafePtr<TestBase> safePtrCopy = nullptr;

	{
		// Constructing a new SafeOwner, should increment the counter
		SafeOwner<Test> safeOwnerOld(deletionQueue, new Test(10, counter));
		REQUIRE(counter == 1);

		// Constructing another SafeOwner, should increment the counter
		auto safeOwner = SafeOwner<Test>(deletionQueue, new Test(5, counter));
		REQUIRE(counter == 2);

		// Moving the old owner to the new one, this should not change the counter, but move the previous content of the new one to the DeletionQueue
		safeOwner = std::move(safeOwnerOld);
		REQUIRE(counter == 2);
		REQUIRE(safeOwner->mX == 10);

		// Construct a SafePtr pointing to safeOwner's content
		safePtr = safeOwner.get();
		// Make a copy of the SafePtr using polymorhpism
		safePtrCopy = safePtr;
	}

	// The owner goes out of scope, however the object should still be in the DeletionQueue and the SafePtr's should be valid
	REQUIRE(counter == 2);
	REQUIRE(safePtr->mX == 10);
	REQUIRE(safePtrCopy->getX() == 10);

	// We clear the DeletionQueue, the objects should be destroyed, the counter should be zero and the SafePtr's set to nullptr
	deletionQueue.clear();
	REQUIRE(counter == 0);
	REQUIRE(safePtr == nullptr);
	REQUIRE(safePtrCopy == nullptr);
}

TEST_CASE("Signals and slots", "[signalslot]")
{
	int x = 0;
	nap::Signal<int&> signal;

	nap::Slot<int&> slot = {
			[](int& x) { x++; }
	};

	signal.connect(slot);
	signal(x);
	REQUIRE(x == 1);

	signal.disconnect(slot);
	signal(x);
	REQUIRE(x == 1);
}

TEST_CASE("Core", "[core]")
{
	/*
	nap::Core core;

    std::string dataFile = nap::utility::getExecutableDir() + "/unit_tests_data/entitystructure.json";
	dataFile = nap::utility::getAbsolutePath(dataFile);

	nap::utility::ErrorState err;
	if (!core.initializeEngine(err))
		FAIL(err.toString());

	if (!core.getResourceManager()->loadFile(dataFile, err))
		FAIL(err.toString());
	*/
}