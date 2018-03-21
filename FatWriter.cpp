#include "Logging.h"
#include "FatWriter.h"
#include <stdexcept>
#include <cassert>

const int SPEED_MULTIPLE = 10;


struct FatWriter::ManageUnit
{
	BufferWriter * pbw;
	int buffer_free_count;
};


FatWriter::FatWriter(int opening_files_limit, int buffer_capacity, int flush_interval_ms)
	: m_file_limit(opening_files_limit)
	, m_initial_capacity(buffer_capacity)
	, m_slow_milliseconds(flush_interval_ms)
	, m_fast_milliseconds(flush_interval_ms / SPEED_MULTIPLE)
{
	if (m_fast_milliseconds < 1)
	{
		throw std::logic_error("flush interval argument is too fast");
	}

#ifdef GLOG
	glog("[%s] OFL:%i IBC:%i SFI(ms):%i FFI(ms):%i\n", __FUNCTION__, m_file_limit, m_initial_capacity, m_slow_milliseconds, m_fast_milliseconds);
#endif // GLOG

	m_bg_thread = std::thread([this]() {
		int n = 0;
		while (!m_exit)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(m_fast_milliseconds));
			++n %= SPEED_MULTIPLE;

			std::lock_guard<std::mutex> lockguard(m_mtx);

			this->maintain_fast_list();

			if (n == 0)
			{
				this->maintain_slow_list();
#ifdef GLOG
				auto nfast = m_fasters.size();
				auto nslow = m_slowers.size();
				if (nfast * 2 > nslow)
				{
					glog("(F/S):(%i/%i)\n", nfast, nslow);
				}
#endif // GLOG
			}
		}
	});
}

FatWriter::~FatWriter()
{
	m_exit = true;

	m_bg_thread.join();

#ifdef GLOG
	auto nfast = m_fasters.size();
	auto nslow = m_slowers.size();
	glog("[%s] (F/S):(%i/%i)\n", __FUNCTION__, nfast, nslow);
#endif // GLOG

	for (auto & i: m_fasters)
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

FormatWriter * FatWriter::create(const std::string & path)
{
	//glog("[%s] %s\n", __FUNCTION__, path.c_str());

	auto ret = new BufferWriter(m_initial_capacity, path); // On failure, it throws a bad_alloc exception.

	m_mtx.lock();
	m_slowers.push_back(ManageUnit{ ret , 0 });
	m_mtx.unlock();

	return static_cast<FormatWriter *>(ret);
}

void FatWriter::maintain_fast_list(void)
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

void FatWriter::maintain_slow_list(void)
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
			throw std::logic_error("unknown buffer state");
			break;
		}

		itr++;
	}
}

void FatWriter::claim_opening_file(void)
{
	if (m_file_count < m_file_limit)
	{
		m_file_count++;
		return;
	}

	BufferWriter * pbw = nullptr;

	/////////////////////////////////////////////////////////////////////////////////

	for (auto & i: m_slowers)
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

	throw std::logic_error("lost opening file unit");
}

void FatWriter::minus_opening_file(void)
{
	m_file_count--;
}
