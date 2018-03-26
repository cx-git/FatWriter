#pragma once

#include <thread>
#include <atomic>
#include <list>
#include <mutex>
#include <fstream>

#include "fatwriter.h"
using namespace fatwriter;


class WriterHubImpl
	: public WriterHub
	, public Logger
{
public:
	static WriterHubImpl * create_instance(
		int opening_files_limit,
		int buffer_capacity,
		int flush_interval_ms,
		std::string logger_path,
		std::string & error_prompt);

	virtual Writer * create(const std::string & path) override;

private:
	explicit WriterHubImpl(
		int opening_files_limit,
		int buffer_capacity,
		int flush_interval_ms,
		std::string logger_path);

	virtual ~WriterHubImpl(void);

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

	std::mutex m_logger_mtx;
	std::ofstream m_logger;

protected:
	// Inherited via Logger
	virtual void info(const char * format, ...) override;
	virtual void error(const char * format, ...) override;
};
