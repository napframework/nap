#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include <utility/fileutils.h>
#include <utility/datetimeutils.h>
#include <audio/utility/safeptr.h>
#include <nap/logger.h>


TEST_CASE("File path transformations", "[fileutils]")
{
	// Extensions
	REQUIRE(nap::utility::getFileExtension("/home/test/one.ext") != ".ext");
	REQUIRE(nap::utility::getFileExtension("/home/test/one.ext") == "ext");
	REQUIRE(nap::utility::getFileExtension("poekie.poes.png") == "png");
	REQUIRE(nap::utility::getFileExtension("tommy.toedel.") == "");
	REQUIRE(nap::utility::getFileExtension("file-name.longextension") == "longextension");

//	REQUIRE(nap::utility::getFileDir("/home/someone//filename.ext") == "/home/someone");
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

TEST_CASE("DateTime Utilities", "[datetime]")
{
	auto currenttime = nap::utility::getCurrentTime();
	auto flc1_launch_str = "2006-03-24 22:30:01.123";
	auto flc1_launch = nap::utility::createTimestamp(2006, 03, 24, 22, 30, 01, 123);

	nap::utility::DateTime flc1_launch_date(flc1_launch);
	REQUIRE(flc1_launch_date.getYear() == 2006);
	REQUIRE(flc1_launch_date.getMonth() == nap::utility::EMonth::March);
	REQUIRE(flc1_launch_date.getDayInTheMonth() == 24);
	REQUIRE(flc1_launch_date.getHour() == 22);
	REQUIRE(flc1_launch_date.getMinute() == 30);
	REQUIRE(flc1_launch_date.getSecond() == 01);
	REQUIRE(flc1_launch_date.getMilliSecond() == 123);

	REQUIRE(nap::utility::timeFormat(flc1_launch) == flc1_launch_str);
}

TEST_CASE("Safe pointers", "[safepointer]")
{
    // Test object that increments a counter when it's constructed and decrements it on destruction
    class Test {
    public:
        Test(int x, int& destructed) : mX(x), mCounter(destructed) { mCounter++; }
        ~Test() { mCounter--; }
        int mX = 0;
        int& mCounter;
    };
    
    int counter = 0; // The counter to count the number of existing objects
    nap::DeletionQueue deletionQueue; // The DeletionQueue used
    nap::SafePtr<Test> safePtr = nullptr;
    nap::SafePtr<Test> safePtrCopy = nullptr;

    {
        // Constructing a new SafeOwner, should increment the counter
        nap::SafeOwner<Test> safeOwnerOld(deletionQueue, new Test(10, counter));
        REQUIRE(counter == 1);
        
        // Constructing another SafeOwner, should increment the counter
        auto safeOwner = nap::SafeOwner<Test>(deletionQueue, new Test(5, counter));
        REQUIRE(counter == 2);
        
        // Moving the old owner to the new one, this should not change the counter, but move the previous content of the new one to the DeletionQueue
        safeOwner = std::move(safeOwnerOld);
        REQUIRE(counter == 2);
        REQUIRE(safeOwner->mX == 10);

        // Construct a SafePtr pointing to safeOwner's content
        safePtr = safeOwner.getSafe();
        // Make a copy of the SafePtr
        safePtrCopy = safePtr;
    }
    
    // The owner goes out of scope, however the object should still be in the DeletionQueue and the SafePtr's should be valid
    REQUIRE(counter == 2);
    REQUIRE(safePtr->mX == 10);
    REQUIRE(safePtrCopy->mX == 10);
    
    // We clear the DeletionQueue, the objects should be destroyed, the counter should be zero and the SafePtr's set to nullptr
    deletionQueue.clear();
    REQUIRE(counter == 0);
    REQUIRE(safePtr == nullptr);
    REQUIRE(safePtrCopy == nullptr);
}
