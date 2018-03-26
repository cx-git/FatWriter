#include "fake_test.h"
#include "fatwriter.h"

#include <chrono>
#include <random>
#include <cstdio>
#include <iomanip>
#include <fstream>
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <vector>
using std::vector;
#include <cmath>
#include <cassert>
#include <thread>
#include <ctime>

#define RET_ERROR -1
#define RET_OK 0

const char LINE_LIB[] = "0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/0123456789,abcdefghijklmnopqrstuvwzxyz.ABCDEFGHIJKLMNOPQRSTUVWXZY?[];!@#$()-+*/";
char LINE_BUF[2048];
const unsigned int FIVE = 5;
const unsigned int TEN = 10;

unsigned int get_seed(void)
{
	return static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
}

bool test_dir(const string & dir)
{
	string path(dir);
	if (path.back() != '/' && path.back() != '\\')
	{
		path.append("/foo.txt");
	}
	else
	{
		path.append("foo.txt");
	}

	auto p = fopen(path.c_str(), "w");
	auto ret = (p != nullptr);
	if (ret)
	{
		fclose(p);
		if (std::remove(path.c_str()) != 0)
		{
			perror(path.c_str());
		}
	}
	
	return ret;
}

bool test_path(const string & path)
{
	auto _path(path);
	_path.append(".tst");

	auto p = fopen(_path.c_str(), "w");
	auto ret = (p != nullptr);
	if (ret)
	{
		fclose(p);
		remove(_path.c_str());
	}

	return ret;
}

const int TIMESTAMP_BUFFER_SIZE = 64;
const char * timestamp(char * cbuf)
{
	auto t = std::time(nullptr);
	std::strftime(cbuf, TIMESTAMP_BUFFER_SIZE, "%x %X ", std::localtime(&t));
	return cbuf;
}

struct FakeRacer
{
	string line;
	int interval_ms;
	int write_times;
	fatwriter::Writer * fw{ nullptr };
};

int fake_test(FatWriterParameter fwp, TestCaseParameter tcp, string test_file_dir, string summary_path, string runtime_path)
{
	if (!::test_dir(test_file_dir))
	{
		cerr << "[ERROR] "  << "Invalid test_file_dir: " << test_file_dir << endl;
		return RET_ERROR;
	}
	if (!::test_path(summary_path))
	{
		cerr << "[ERROR] " << "Invalid summary_path: " << summary_path << endl;
		return RET_ERROR;
	}
	if (test_file_dir.back() != '/' && test_file_dir.back() != '\\')
	{
		test_file_dir.append("/");
	}

	char cbuf_timestamp[TIMESTAMP_BUFFER_SIZE];
	std::ofstream ofs(summary_path, std::ios_base::out | std::ios_base::app);
	ofs << "==============================================================" << endl;
	ofs << timestamp(cbuf_timestamp) << "fake test start" << endl;

	vector<string> file_paths;
	vector< vector<FakeRacer> > file_racers;

	std::default_random_engine rand_int(get_seed());

	for (int i = 0; i < tcp.file_count; i++)
	{
		auto file_path = test_file_dir + std::to_string(i);
		file_paths.push_back(file_path);

		vector<FakeRacer> racers;
		auto racer_count = std::max(rand_int() % tcp.racer_count_bound, FIVE);
		for (size_t j = 0; j < racer_count; j++)
		{
			FakeRacer racer;

			racer.write_times = std::max(rand_int() % tcp.write_times_bound, FIVE);
			racer.interval_ms = std::max(rand_int() % tcp.seconds_bound, FIVE) * 1000 / racer.write_times;

			auto line_length = std::max(rand_int() % tcp.line_byte_bound, TEN);
			strncpy(LINE_BUF, LINE_LIB, line_length);
			LINE_BUF[line_length] = '\0';
			racer.line.assign(LINE_BUF);

			assert(racer.line.size() == line_length);

			racers.push_back(racer);
		}
		file_racers.push_back(racers);
	}

	for (size_t i = 0; i < file_paths.size(); i++)
	{
		ofs << file_paths[i] << endl;
		for (size_t j = 0; j < file_racers[i].size(); j++)
		{
			ofs << std::setw(5) << file_racers[i][j].line.size() << std::setw(5) << file_racers[i][j].interval_ms << std::setw(5) << file_racers[i][j].write_times << endl;
		}
	}

	string error_prompt;
	auto pfw = fatwriter::create_writer_hub(fwp.opening_files_limit, fwp.buffer_capacity, fwp.flush_interval_ms, runtime_path, error_prompt);
	if (pfw == nullptr)
	{
		cerr << "[ERROR] " << "create_writer_hub: " << error_prompt << endl;
		return RET_ERROR;
	}

	vector<std::thread> thrpool;
	for (size_t i = 0; i < file_paths.size(); i++)
	{
		auto pbw = pfw->create(file_paths[i]);

		for (size_t j = 0; j < file_racers[i].size(); j++)
		{
			file_racers[i][j].fw = pbw;

			thrpool.push_back(std::thread([&ofs](string fp, FakeRacer fr){
				for (int t = 0; t < fr.write_times; t++)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(fr.interval_ms));
					if (!fr.fw->printf("%s\n", fr.line.c_str()))
					{
						ofs << "[ERROR]" << fp << ": " << fr.line << endl;
					}
				}
			}, file_paths[i], file_racers[i][j]));
		}
	}

	for (size_t i = 0; i < thrpool.size(); i++)
	{
		//cout << "T" << i << " joined." << endl;
		thrpool[i].join();
	}

	pfw.reset();
	pfw = nullptr;

	vector<int> file_sizes;
	for (const auto & fp: file_paths)
	{
		auto ch_count = 0;
		auto ln_count = 0;
		int ch;
		auto p = fopen(fp.c_str(), "r");
		if (p == nullptr)
		{
			file_sizes.push_back(0);
		}
		else
		{
			while ((ch = fgetc(p)) != EOF)
			{
				if (ch == '\n')
				{
					ln_count++;
				}
				else
				{
					ch_count++;
				}
			}
			fclose(p);

			file_sizes.push_back(ch_count);

			if (std::remove(fp.c_str()) != 0)
			{
				perror(fp.c_str());
			}
		}
	}

	auto ret = RET_OK;
	
	ofs << "--------------------------------------------------------------" << endl;
	for (size_t i = 0; i < file_racers.size(); i++)
	{
		auto ch_count = 0;
		for (size_t j = 0; j < file_racers[i].size(); j++)
		{
			ch_count += file_racers[i][j].line.size() * file_racers[i][j].write_times;
		}

		if (ch_count != file_sizes[i])
		{
			ret = RET_ERROR;
			cout << file_paths[i] << ": " << ch_count << " - " << file_sizes[i] << endl;
			ofs << file_paths[i] << ": " << ch_count << " - " << file_sizes[i] << " = " << ch_count - file_sizes[i] << endl;
		}
	}
	ofs << "--------------------------------------------------------------" << endl;

	ofs << timestamp(cbuf_timestamp) << "fake test end" << endl;
	return ret;
}
