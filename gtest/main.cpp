#include "gtest\gtest.h"
#include "FakeTest.h"
#include "FatWriter.h"
#include <exception>
#include <cstdio>

TEST(FatWriter, CreateRelease)
{
	bool ret;

	try {
		auto p = new FatWriter(10, 1024, 500);
		p->create("test/01.txt");
		p->create("test/02.txt");
		p->create("test/03.txt");
		p->create("test/04.txt");
		p->create("test/05.txt");
		p->create("test/06.txt");
		delete p;
		ret = true;
	}
	catch (std::exception & e)
	{
		perror(e.what());
		ret = false;
	}

	ASSERT_TRUE(ret);
}

TEST(FatWriter, Fake)
{
	try {
		ASSERT_EQ(0, fake_test(FatWriterParameter{ 1, 64, 1000 }, TestCaseParameter{ 10, 50, 512, 500, 30 }, "H:/test/fatwriter/tmp", "H:/test/fatwriter/fake.txt"));
	}
	catch (std::exception & e)
	{
		perror(e.what());
		ASSERT_TRUE(false);
	}
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
