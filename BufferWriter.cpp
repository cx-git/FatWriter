#include "Logging.h"

#include "BufferWriter.h"
#include <cstdarg>
#include <stdexcept>


BufferWriter::BufferWriter(const int capacity, const std::string & path)
	: m_fs_path(path)
	, m_cbuf_capacity(capacity)
	, m_cbuf_size(static_cast<int>(capacity * 0.7))
{
	m_cbuf_ptr = new char[m_cbuf_capacity];
	if (m_cbuf_ptr == nullptr)
	{
		throw std::bad_alloc();
	}
}

BufferWriter::~BufferWriter()
{
	m_cbuf_locker.lock();

	if (m_cbuf_length > 0)
	{
		if (m_fs_ptr == nullptr)
		{
			m_fs_ptr = fopen(m_fs_path.c_str(), "a");
		}

		if (m_fs_ptr != nullptr)
		{
			fprintf(m_fs_ptr, "%s", m_cbuf_ptr);
			m_cbuf_length = 0;
		}
		else
		{
			glog("ERROR ! [%s] open file stream error: %s\n", __FUNCTION__, m_fs_path.c_str());
		}
	}

	m_cbuf_locker.unlock();

	if (m_fs_ptr != nullptr)
	{
		fclose(m_fs_ptr);
		m_fs_ptr = nullptr;
	}
}

std::string BufferWriter::get_file_path(void)
{
	return m_fs_path;
}

void BufferWriter::printf(const char * format, ...)
{
	std::lock_guard<std::mutex> lockguard(m_cbuf_locker);

	auto n = m_cbuf_capacity - m_cbuf_length;

	va_list args;

	va_start(args, format);
	auto r = vsnprintf(m_cbuf_ptr + m_cbuf_length, n, format, args);
	va_end(args);

	if (r >= 0 && r < n)
	{
		m_cbuf_length += r;

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
	else
	{
		m_cbuf_state = BS_EXPAND;

		m_cbuf_capacity *= 2;
		m_cbuf_size = static_cast<int>(m_cbuf_capacity * 0.7);

		auto pcbuf = new char[m_cbuf_capacity];
		if (pcbuf == nullptr)
		{
			throw std::bad_alloc();
		}

		glog("[%s] expand buffer: %i\n", m_fs_path.c_str(), m_cbuf_capacity);

		std::strcpy(pcbuf, m_cbuf_ptr);
		delete [] m_cbuf_ptr;
		m_cbuf_ptr = pcbuf;

		n = m_cbuf_capacity - m_cbuf_length;

		va_start(args, format);
		auto r = vsnprintf(m_cbuf_ptr + m_cbuf_length, n, format, args);
		va_end(args);

		if (r >= 0 && r < n)
		{
			m_cbuf_length += r;
		}
		else
		{
			throw std::overflow_error(m_fs_path.c_str());
		}
	}
}

BufferStateRet BufferWriter::flush(void)
{
	BufferStateRet ret;

	std::lock_guard<std::mutex> lockguard(m_cbuf_locker);

	ret.former = m_cbuf_state;

	if (m_cbuf_length > 0 && m_fs_ptr != nullptr)
	{
		auto r = fprintf(m_fs_ptr, "%s", m_cbuf_ptr);
		if (r != m_cbuf_length)
		{
			throw std::runtime_error(__FUNCTION__);
		}
		m_cbuf_length = 0;
		m_cbuf_ptr[0] = '\0';
		m_cbuf_state = BS_FREE;
	}

	ret.lastest = m_cbuf_state;

	return ret;
}

bool BufferWriter::isopen(void)
{
	return m_fs_ptr != nullptr;
}

int BufferWriter::fsopen(void)
{
	if (m_fs_ptr == nullptr)
	{
		m_fs_ptr = fopen(m_fs_path.c_str(), "a");
		if (m_fs_ptr == nullptr)
		{
			return errno;
		}
		else
		{
			return 0;
		}
	}

	return 0;
}

int BufferWriter::fsclose(void)
{
	auto ret = 0;

	if (m_fs_ptr != nullptr)
	{
		ret = fclose(m_fs_ptr);
		m_fs_ptr = nullptr;
	}

	return ret;
}
