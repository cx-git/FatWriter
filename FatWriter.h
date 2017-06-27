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
	explicit FatWriter(int file_limit, int capacity, int slow_milliseconds);
	~FatWriter(void);

	virtual FormatWriter * create(const std::string & path);

private:
	// [MultThread Unsafe] called by background thread
	void maintain_fast_list(void);

	// [MultThread Unsafe] called by background thread
	void maintain_slow_list(void);

	// [MultThread Unsafe] called by background thread
	void apply_file_stream(void);

	// [MultThread Unsafe] called by background thread
	void free_file_stream(void);

private:
	std::mutex m_locker;

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
