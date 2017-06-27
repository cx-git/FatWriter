/*
test(n)
--thread(3)
----token<random>
----span<random>
----times<random>
----count
*/

#include "FatWriter.h"
#include <list>
#include <cstring>
#include <cstdio>
#include <chrono>
#include <thread>
#include <random>
#include <string>
#include <stdexcept>


const int file_limit = 100;
const int capacity = BUFSIZ;
const int slow_milliseconds = 800;

const int n_test_unit = 300;
const int n_thread_unit = 3;

const int mod_times = 100;

char atomic_token[] = "0123456789";
const int mod_token = 10;

const int atomic_span_ms = 100;
const int mod_span = 30;


struct ThreadUnit
{
	unsigned int seed{ 0 };
	unsigned int ch_count{ 0 };
	unsigned int ln_count{ 0 };
};

struct TestUnit
{
	std::list<ThreadUnit> thread_units;
	FormatWriter * fw{ nullptr };
	std::string path;
};

unsigned int get_seed(void)
{
	return static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
}

int main(void)
{
	auto fatwriter_ptr = new FatWriter(file_limit, capacity, slow_milliseconds);

	std::list<TestUnit> test_units;

	std::list<std::thread> ths;

	std::default_random_engine re(get_seed());

	printf("Create...\n");
	for (size_t i = 0; i < n_test_unit; i++)
	{
		TestUnit test_unit;
		
		char cbuf[BUFSIZ];
		sprintf(cbuf, "test/fw_test_%i.txt", i);
		test_unit.path.assign(cbuf);
		test_unit.fw = fatwriter_ptr->create(cbuf);

		for (size_t j = 0; j < n_thread_unit; j++)
		{
			ThreadUnit thread_unit;
			thread_unit.seed = re();

			test_unit.thread_units.push_back(thread_unit);
		}

		test_units.push_back(test_unit);
	}

	printf("Run...\n");
	for (auto & i: test_units)
	{
		for (auto & j: i.thread_units)
		{
			ths.push_back(std::thread([&j](FormatWriter * fw) {
				std::default_random_engine re(j.seed);
				auto times = re() % mod_times;
				for (size_t p = 0; p < times; p++)
				{
					auto m = re() % mod_span + 1;
					std::this_thread::sleep_for(std::chrono::milliseconds(atomic_span_ms * m));

					char cbuf[BUFSIZ]{ "" };
					m = re() % mod_token + 1;
					for (size_t q = 0; q < m; q++)
					{
						strcat(cbuf, atomic_token);
					}

					j.ch_count += strlen(cbuf);
					j.ln_count++;
					fw->printf("%s\n", cbuf);
				}
			}, i.fw));
		}
	}

	printf("Join...\n");
	auto a = 0;
	auto aa = ths.size();
	for (auto & i : ths)
	{
		i.join();
		//printf("%i/%i\n", ++a, aa);
	}

	printf("Release...\n");
	delete fatwriter_ptr;
	fatwriter_ptr = nullptr;

	printf("Validate+Clean...\n");
	for (auto & i : test_units)
	{
		auto total_ch_count = 0;
		auto total_ln_count = 0;
		for (auto & j : i.thread_units)
		{
			total_ch_count += j.ch_count;
			total_ln_count += j.ln_count;
		}

		auto ch_count = 0;
		auto ln_count = 0;
		int ch;
		auto fp = fopen(i.path.c_str(), "r");
		if (fp == nullptr) perror("Error opening file");
		while ((ch = fgetc(fp)) != EOF)
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
		fclose(fp);

		std::remove(i.path.c_str());

		if (ch_count != total_ch_count || ln_count != total_ln_count)
		{
			fprintf(stderr, "%s inconsistent: [ch]%i/%i [ln]%i/%i\n", i.path.c_str(), ch_count, total_ch_count, ln_count, total_ln_count);
		}
		else
		{
			//fprintf(stderr, "%s: [ch]%i/%i [ln]%i/%i\n", i.path.c_str(), ch_count, total_ch_count, ln_count, total_ln_count);
		}
	}

	printf("Done...\n");

	getc(stdin);

	return 0;
}