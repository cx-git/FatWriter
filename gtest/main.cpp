#include "gtest\gtest.h"
#include "fake_test.h"
#include "fatwriter.h"
#include <exception>
#include <cstdio>
#include <fstream>
#include <string>
using std::string;


TEST(fatwriter, wrong_init)
{
	string error_prompt;

	ASSERT_EQ(nullptr, fatwriter::create_writer_hub(0, 1024, 1000, "runtime.log", error_prompt));
	ASSERT_EQ(error_prompt.compare("opening_files_limit must GTR zero"), 0);

	ASSERT_EQ(nullptr, fatwriter::create_writer_hub(10, 50, 1000, "runtime.log", error_prompt));
	ASSERT_EQ(error_prompt.compare("buffer_capacity must GTE 64 bytes"), 0);

	ASSERT_EQ(nullptr, fatwriter::create_writer_hub(10, 1024, 5, "runtime.log", error_prompt));
	ASSERT_EQ(error_prompt.compare("flush_interval_ms must GTE 10 ms"), 0);

	ASSERT_EQ(nullptr, fatwriter::create_writer_hub(10, 1024, 1000, "FOO:/bar.log", error_prompt));
}

TEST(fatwriter, fake_busy)
{
	const char summary_path[] = "H:/test/fatwriter/fake_summary.txt";
	const char runtime_path[] = "H:/test/fatwriter/fake_runtime.txt";
	ASSERT_EQ(0, ::fake_test(FatWriterParameter{ 1, 64, 1000 }, TestCaseParameter{ 10, 50, 512, 500, 10 }, "H:/test/fatwriter/tmp", summary_path, runtime_path));
}

TEST(fatwriter, fake_easy)
{
	const char summary_path[] = "H:/test/fatwriter/fake_summary.txt";
	const char runtime_path[] = "H:/test/fatwriter/fake_runtime.txt";
	ASSERT_EQ(0, ::fake_test(FatWriterParameter{ 5, 256, 100 }, TestCaseParameter{ 10, 50, 512, 500, 60 }, "H:/test/fatwriter/tmp", summary_path, runtime_path));
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
