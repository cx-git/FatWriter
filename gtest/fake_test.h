#pragma once

#include <string>
using std::string;

struct FatWriterParameter
{
	int opening_files_limit;
	int buffer_capacity;
	int flush_interval_ms;
};

struct TestCaseParameter
{
	int file_count;
	int racer_count_bound;
	int line_byte_bound;
	int write_times_bound;
	int seconds_bound;
};

int fake_test(FatWriterParameter fwp, TestCaseParameter tcp, string test_file_dir, string summary_path, string runtime_path);
