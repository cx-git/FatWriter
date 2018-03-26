#include "writer_hub.h"
#include "writer.h"

#include <cassert>
#include <ctime>
#include <cstdarg>

using std::endl;


struct WriterHubImpl::ManageUnit
{
	WriterImpl * pbw;
	int buffer_free_count;
};

std::shared_ptr<WriterHub> fatwriter::create_writer_hub(int opening_files_limit, int buffer_capacity, int flush_interval_ms, std::string logger_path, std::string & error_prompt)
{
	return std::shared_ptr<WriterHub>(static_cast<WriterHub *>(WriterHubImpl::create_instance(opening_files_limit, buffer_capacity, flush_interval_ms, logger_path, error_prompt)));
}

WriterHubImpl * WriterHubImpl::create_instance(int opening_files_limit, int buffer_capacity, int flush_interval_ms, std::string logger_path, std::string & error_prompt)
{
	if (opening_files_limit <= 0)
	{
		error_prompt.assign("opening_files_limit must GTR zero");
		return nullptr;
	}

	const int BUFFER_CAPACITY_THRESHOLD = 64;
	if (buffer_capacity < BUFFER_CAPACITY_THRESHOLD)
	{
		error_prompt.assign("buffer_capacity must GTE " + std::to_string(BUFFER_CAPACITY_THRESHOLD) + " bytes");
		return nullptr;
	}

	const int FLUSH_INTERVAL_MS = 10;
	if (flush_interval_ms < FLUSH_INTERVAL_MS)
	{
		error_prompt.assign("flush_interval_ms must GTE " + std::to_string(FLUSH_INTERVAL_MS) + " ms");
		return nullptr;
	}

	auto fp = fopen(logger_path.c_str(), "r");
	if (fp == nullptr)
	{
		fp = fopen(logger_path.c_str(), "w");
		if (fp == nullptr)
		{
			error_prompt.assign("logger_path error: " + logger_path);
			return nullptr;
		}
	}
	fclose(fp);

	return new WriterHubImpl(opening_files_limit, buffer_capacity, flush_interval_ms, logger_path);
}

Writer * WriterHubImpl::create(const std::string & path)
{
	WriterImpl * pw;
	try
	{
		pw = new WriterImpl(m_initial_capacity, path, this);  // On failure, it throws a bad_alloc exception.
	}
	catch (const std::exception & e)
	{
		this->error("[%s] %s", __FUNCTION__, e.what());
		return nullptr;
	}

	m_mtx.lock();
	m_slowers.push_back(ManageUnit{ pw , 0 });
	m_mtx.unlock();

	return static_cast<Writer *>(pw);
}

const int SPEED_MULTIPLE = 10;

WriterHubImpl::WriterHubImpl(int opening_files_limit, int buffer_capacity, int flush_interval_ms, std::string logger_path)
	: m_file_limit(opening_files_limit)
	, m_initial_capacity(buffer_capacity)
	, m_slow_milliseconds(flush_interval_ms)
	, m_fast_milliseconds(flush_interval_ms / SPEED_MULTIPLE)
	, m_logger(logger_path, std::ios_base::app | std::ios_base::out)
{
	this->info("[%s] OFL:%i IBC:%i SFI(ms):%i FFI(ms):%i", __FUNCTION__, m_file_limit, m_initial_capacity, m_slow_milliseconds, m_fast_milliseconds);

	m_bg_thread = std::thread([this]() {
		try
		{
			int n = 0;
			while (!m_exit)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(m_fast_milliseconds));
				++n %= SPEED_MULTIPLE;
				this->m_mtx.lock();
				this->maintain_fast_list();
				if (n == 0)
				{
					this->maintain_slow_list();
					auto nfast = m_fasters.size();
					auto nslow = m_slowers.size();

					this->m_mtx.unlock();

					if (nfast * 2 > nslow)
					{
						this->info("(F/S):(%i/%i)", nfast, nslow);
					}
				}
				else
				{
					this->m_mtx.unlock();
				}
			}
		}
		catch (const std::exception& e)
		{
			this->error("BGT.RUNTIME: %s", e.what());
			std::exit(-1);
		}
	});
}

WriterHubImpl::~WriterHubImpl(void)
{
	m_exit = true;
	m_bg_thread.join();
	auto nfast = m_fasters.size();
	auto nslow = m_slowers.size();
	this->info("[%s] (F/S):(%i/%i)", __FUNCTION__, nfast, nslow);

	for (auto & i : m_fasters)
	{
		delete i.pbw;
		i.pbw = nullptr;
	}
	m_fasters.clear();

	for (auto & i : m_slowers)
	{
		delete i.pbw;
		i.pbw = nullptr;
	}
	m_slowers.clear();

	m_file_count = 0;
}

void WriterHubImpl::maintain_fast_list(void)
{
	auto itr = m_fasters.begin();
	while (itr != m_fasters.end())
	{
		if (itr->buffer_free_count > SPEED_MULTIPLE)
		{
			itr->buffer_free_count = 0;
			m_slowers.push_back(*itr);
			itr = m_fasters.erase(itr);
			continue;
		}

		auto ret = itr->pbw->flush();
		if (ret.lastest != BS_FREE)
		{
			this->claim_opening_file();
			itr->pbw->open_file();
			ret = itr->pbw->flush();
		}
		assert(ret.lastest == BS_FREE);
		if (ret.former == BS_FREE)
		{
			itr->buffer_free_count++;
		}
		else
		{
			itr->buffer_free_count = 0;
		}

		itr++;
	}
}

void WriterHubImpl::maintain_slow_list(void)
{
	auto itr = m_slowers.begin();
	while (itr != m_slowers.end())
	{
		auto ret = itr->pbw->flush();
		if (ret.lastest != BS_FREE)
		{
			this->claim_opening_file();
			itr->pbw->open_file();
			ret = itr->pbw->flush();
		}
		assert(ret.lastest == BS_FREE);
		switch (ret.former)
		{
		case BS_FREE:
			itr->buffer_free_count++;
			if (itr->buffer_free_count > SPEED_MULTIPLE && itr->pbw->is_file_open())
			{
				itr->pbw->close_file();
				this->minus_opening_file();
			}
			break;

		case BS_FILL:
			itr->buffer_free_count = 0;
			break;

		case BS_FULL:
		case BS_EXPAND:
			itr->buffer_free_count = 0;
			m_fasters.push_back(*itr);
			itr = m_slowers.erase(itr);
			continue;
			break;

		default:
			throw std::runtime_error("unknown buffer state");
			break;
		}

		itr++;
	}
}

void WriterHubImpl::claim_opening_file(void)
{
	if (m_file_count < m_file_limit)
	{
		m_file_count++;
		return;
	}

	WriterImpl * pbw = nullptr;

	/////////////////////////////////////////////////////////////////////////////////

	for (auto & i : m_slowers)
	{
		if (i.pbw->is_file_open())
		{
			if (pbw == nullptr)
			{
				pbw = i.pbw;
			}

			if (i.buffer_free_count > 0)
			{
				i.pbw->close_file();
				return;
			}
		}
	}

	if (pbw != nullptr)
	{
		pbw->close_file();
		return;
	}

	/////////////////////////////////////////////////////////////////////////////////

	for (auto & i : m_fasters)
	{
		if (i.pbw->is_file_open())
		{
			if (pbw == nullptr)
			{
				pbw = i.pbw;
			}

			if (i.buffer_free_count > 0)
			{
				i.pbw->close_file();
				return;
			}
		}
	}

	if (pbw != nullptr)
	{
		pbw->close_file();
		return;
	}

	/////////////////////////////////////////////////////////////////////////////////

	throw std::runtime_error("lost opening file unit");
}

void WriterHubImpl::minus_opening_file(void)
{
	m_file_count--;
}

void WriterHubImpl::info(const char * format, ...)
{
	char timestamp[32];
	auto t = std::time(nullptr);
	std::strftime(timestamp, 32, "[%F %T]", std::localtime(&t));

	char cbuf[BUFSIZ];
	va_list args;
	va_start(args, format);
	auto n = vsnprintf(cbuf, BUFSIZ, format, args);
	va_end(args);

	auto m = n - 1;
	if (cbuf[m] == '\n')
	{
		cbuf[m] = '\0';
	}

	m_logger_mtx.lock();
	m_logger << timestamp << " [INFO] " << cbuf << endl;
	m_logger_mtx.unlock();
}

void WriterHubImpl::error(const char * format, ...)
{
	char timestamp[32];
	auto t = std::time(nullptr);
	std::strftime(timestamp, 32, "[%F %T]", std::localtime(&t));

	char cbuf[BUFSIZ];
	va_list args;
	va_start(args, format);
	auto n = vsnprintf(cbuf, BUFSIZ, format, args);
	va_end(args);

	auto m = n - 1;
	if (cbuf[m] == '\n')
	{
		cbuf[m] = '\0';
	}

	m_logger_mtx.lock();
	m_logger << timestamp << " [ERROR] " << cbuf << endl;
	m_logger_mtx.unlock();
}

