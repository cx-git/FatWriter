#include "Logging.h"
#include "FatWriter.h"
#include <stdexcept>

const int SPEED_MULTIPLE = 10;

struct FatWriter::ManageUnit
{
	BufferWriter * handler;
	int free_count{ 0 };
};


FatWriter::FatWriter(int file_limit, int capacity, int slow_milliseconds)
	: m_file_limit(file_limit)
	, m_initial_capacity(capacity)
	, m_slow_milliseconds(slow_milliseconds)
	, m_fast_milliseconds(slow_milliseconds / SPEED_MULTIPLE)
{
	if (m_fast_milliseconds < 1)
	{
		throw std::logic_error("too fast");
	}

#ifdef GLOG
	glog("[%s] FL:%i IC:%i Sms:%i Fms:%i\n", __FUNCTION__, m_file_limit, m_initial_capacity, m_slow_milliseconds, m_fast_milliseconds);
#endif // GLOG

	m_bg_thread = std::thread([this]() {
		int n = 0;
		while (!m_exit)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(m_fast_milliseconds));
			++n %= SPEED_MULTIPLE;

			std::lock_guard<std::mutex> lockguard(m_locker);

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

	auto nfast = m_fasters.size();
	auto nslow = m_slowers.size();

	for (auto & i: m_fasters)
	{
		delete i.handler;
		i.handler = nullptr;
	}
	m_fasters.clear();

	for (auto & i : m_slowers)
	{
		delete i.handler;
		i.handler = nullptr;
	}
	m_slowers.clear();

#ifdef GLOG
	glog("[%s] (F/S):(%i/%i)\n", __FUNCTION__, nfast, nslow);
#endif // GLOG
}

FormatWriter * FatWriter::create(const std::string & path)
{
	//glog("[%s] %s\n", __FUNCTION__, path.c_str());

	ManageUnit unit;

	unit.handler = new BufferWriter(m_initial_capacity, path);
	if (unit.handler == nullptr)
	{
		throw std::runtime_error(__FUNCTION__);
	}

	std::lock_guard<std::mutex> lockguard(m_locker);

	m_slowers.push_back(unit);

	return static_cast<FormatWriter *>(unit.handler);
}

void FatWriter::maintain_fast_list(void)
{
	auto & itr = m_fasters.begin();
	while (itr != m_fasters.end())
	{
		if (itr->free_count > SPEED_MULTIPLE)
		{
			//glog("    slow: %s\n", itr->handler->get_file_path().c_str());
			itr->free_count = 0;
			m_slowers.push_back(*itr);
			itr = m_fasters.erase(itr);
			continue;
		}

		auto ret = itr->handler->flush();

		if (ret.lastest != BS_FREE)
		{
			this->apply_file_stream();

			if (itr->handler->fsopen() != 0)
			{
				this->free_file_stream();
				throw std::runtime_error(itr->handler->get_file_path().c_str());
			}

			//glog("    open file stream: %s\n", itr->handler->get_file_path().c_str());

			ret = itr->handler->flush();
		}

		if (ret.former == BS_FREE)
		{
			itr->free_count++;
		}
		else
		{
			itr->free_count = 0;
		}

		itr++;
	}
}

void FatWriter::maintain_slow_list(void)
{
	//glog("[%s]\n", __FUNCTION__);

	auto & itr = m_slowers.begin();
	while (itr != m_slowers.end())
	{
		auto ret = itr->handler->flush();

		if (ret.lastest != BS_FREE)
		{
			this->apply_file_stream();

			if (itr->handler->fsopen() != 0)
			{
				throw std::runtime_error(itr->handler->get_file_path().c_str());
			}

			//glog("    open file stream: %s\n", itr->handler->get_file_path().c_str());

			ret = itr->handler->flush();
		}

		switch (ret.former)
		{
		case BS_FREE:
			itr->free_count++;
			if (itr->free_count > SPEED_MULTIPLE)
			{
				if (itr->handler->isopen())
				{
					if (itr->handler->fsclose() != 0)
					{
						throw std::runtime_error(itr->handler->get_file_path().c_str());
					}

					//glog("    close file stream: %s\n", itr->handler->get_file_path().c_str());

					this->free_file_stream();
				}
			}
			break;

		case BS_FILL:
			itr->free_count = 0;
			break;

		case BS_FULL:
		case BS_EXPAND:
			//glog("    fast: %s\n", itr->handler->get_file_path().c_str());
			itr->free_count = 0;
			m_fasters.push_back(*itr);
			itr = m_slowers.erase(itr);
			continue;
			break;

		default:
			break;
		}

		itr++;
	}
}

void FatWriter::apply_file_stream(void)
{
	if (m_file_count < m_file_limit)
	{
		m_file_count++;
		return;
	}

	/////////////////////////////////////////////////////////////////////////////////

	BufferWriter * first_open_ptr = nullptr;
	for (auto & i: m_slowers)
	{
		if (i.handler->isopen())
		{
			if (first_open_ptr == nullptr)
			{
				first_open_ptr = i.handler;
			}

			if (i.free_count > 0)
			{
				if (i.handler->fsclose() != 0)
				{
					throw std::runtime_error(i.handler->get_file_path().c_str());
				}

				//glog("[%s] swap from free slower: %s\n", __FUNCTION__, i.handler->get_file_path().c_str());

				return;
			}
		}
	}

	if (first_open_ptr != nullptr)
	{
		if (first_open_ptr->fsclose() != 0)
		{
			throw std::runtime_error(first_open_ptr->get_file_path().c_str());
		}

		//glog("[%s] swap from busy slower: %s\n", __FUNCTION__, first_open_ptr->get_file_path().c_str());

		return;
	}

	/////////////////////////////////////////////////////////////////////////////////

	for (auto & i : m_fasters)
	{
		if (i.handler->isopen())
		{
			if (first_open_ptr == nullptr)
			{
				first_open_ptr = i.handler;
			}

			if (i.free_count > 0)
			{
				if (i.handler->fsclose() != 0)
				{
					throw std::runtime_error(i.handler->get_file_path().c_str());
				}

				//glog("[%s] swap from free faster: %s\n", __FUNCTION__, i.handler->get_file_path().c_str());

				return;
			}
		}
	}

	if (first_open_ptr != nullptr)
	{
		if (first_open_ptr->fsclose() != 0)
		{
			throw std::runtime_error(first_open_ptr->get_file_path().c_str());
		}

		//glog("[%s] swap from busy faster: %s, %i/%i\n", __FUNCTION__, first_open_ptr->get_file_path().c_str(), m_fasters.size(), m_slowers.size());

		return;
	}

	/////////////////////////////////////////////////////////////////////////////////

	throw std::logic_error(__FUNCTION__);
}

void FatWriter::free_file_stream(void)
{
	//glog("[%s]\n", __FUNCTION__);

	m_file_count--;
}
