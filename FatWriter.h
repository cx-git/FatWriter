#pragma once

#include "BufferWriter.h"
#include <list>
#include <thread>
#include <atomic>

class WriterCreater
{
public:
	virtual FormatWriter * create(const std::string & path) = 0;
};

class FatWriter
	: public WriterCreater
{
public:
	explicit FatWriter(int opening_files_limit, int buffer_capacity, int flush_interval_ms);
	virtual ~FatWriter(void);

	virtual FormatWriter * create(const std::string & path);

private:
	void maintain_fast_list(void);
	void maintain_slow_list(void);
	void claim_opening_file(void);
	void minus_opening_file(void);

private:
	std::mutex m_mtx;

	struct ManageUnit;

	std::list<ManageUnit> m_fasters;
	std::list<ManageUnit> m_slowers;

	std::thread m_bg_thread;
	std::atomic<bool> m_exit{ false };

	int m_file_count{ 0 };

	int m_file_limit;
	int m_initial_capacity;
	int m_slow_milliseconds;
	int m_fast_milliseconds;
};
