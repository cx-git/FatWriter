#include "Logging.h"

#include "BufferWriter.h"
#include <cstdarg>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <cerrno>


BufferWriter::BufferWriter(const int capacity, const std::string & path)
	: m_cbuf_size(static_cast<int>(capacity * 0.7))
	, m_cbuf_capacity(capacity)
	, m_file_path(path)
{
	m_cbuf = new char[m_cbuf_capacity];  // On failure, it throws a bad_alloc exception.
}

BufferWriter::~BufferWriter()
{
	m_cbuf_locker.lock();

	if (m_cbuf_length > 0)
	{
		if (m_file == nullptr)
		{
			m_file = fopen(m_file_path.c_str(), "a");
			/*if (m_file == nullptr)
			{
				throw std::runtime_error("std::fopen(): " + m_file_path);
			}*/
		}

		if (fprintf(m_file, "%s", m_cbuf) < 0)
		{
			/*throw std::runtime_error("std::fprintf(): " + m_file_path + ": " + m_cbuf);*/
		}
		m_cbuf_length = 0;
	}
	delete[] m_cbuf;

	m_cbuf_locker.unlock();

	if (m_file != nullptr)
	{
		if (fclose(m_file) != 0)
		{
			/*throw std::runtime_error("std::fclose(): " + m_file_path);*/
		}
		m_file = nullptr;
	}
}

void BufferWriter::printf(const char * format, ...)
{
	std::lock_guard<std::mutex> lockguard(m_cbuf_locker);
	do
	{
		auto n = m_cbuf_capacity - m_cbuf_length;
		va_list args;
		va_start(args, format);
		auto r = vsnprintf(m_cbuf + m_cbuf_length, n, format, args);
		va_end(args);
		if (r >= 0 && r < n)
		{
			m_cbuf_length += r;

			if (m_cbuf_state != BS_EXPAND)
			{
				if (m_cbuf_length == 0)
				{
					m_cbuf_state = BS_FREE;
				}
				else if (m_cbuf_length < m_cbuf_size)
				{
					m_cbuf_state = BS_FILL;
				}
				else
				{
					m_cbuf_state = BS_FULL;
				}
			}

			break;
		}
		else
		{
			m_cbuf[m_cbuf_length] = '\0';
			m_cbuf_state = BS_EXPAND;
			m_cbuf_capacity *= 2;
			m_cbuf_size = static_cast<int>(m_cbuf_capacity * 0.7);
			auto cbuf = new char[m_cbuf_capacity]; // On failure, it throws a bad_alloc exception.
#ifdef GLOG
			glog("[%s] expand buffer: %i\n", m_file_path.c_str(), m_cbuf_capacity);
#endif
			std::strcpy(cbuf, m_cbuf);
			delete[] m_cbuf;
			m_cbuf = cbuf;
		}
	} while (true);
}

BufferStateRet BufferWriter::flush(void)
{
	BufferStateRet ret;

	std::lock_guard<std::mutex> lockguard(m_cbuf_locker);

	ret.former = m_cbuf_state;

	if (m_cbuf_length > 0 && m_file != nullptr)
	{
		if (fprintf(m_file, "%s", m_cbuf) != m_cbuf_length)
		{
			throw std::runtime_error("std::fprintf(): " + m_file_path + ": " + m_cbuf);
		}
		m_cbuf_length = 0;
		m_cbuf[0] = '\0';
		m_cbuf_state = BS_FREE;
	}

	ret.lastest = m_cbuf_state;

	return ret;
}

bool BufferWriter::is_file_open(void)
{
	return m_file != nullptr;
}

void BufferWriter::open_file(void)
{
	if (m_file == nullptr)
	{
		m_file = fopen(m_file_path.c_str(), "a");
		if (m_file == nullptr)
		{
			throw std::runtime_error("std::fopen(): " + m_file_path);
		}
	}
}

void BufferWriter::close_file(void)
{
	if (m_file != nullptr)
	{
		if (fclose(m_file) != 0)
		{
			throw std::runtime_error("std::fclose(): " + m_file_path);
		}
		m_file = nullptr;
	}
}
