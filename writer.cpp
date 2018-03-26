#include "writer.h"

#include <cstdarg>


WriterImpl::WriterImpl(const int capacity, const std::string & path, Logger * logger)
	: m_cbuf_size(static_cast<int>(capacity * 0.7))
	, m_cbuf_capacity(capacity)
	, m_file_path(path)
	, m_logger(logger)
{
	this->create_buffer();
}

WriterImpl::~WriterImpl(void)
{
	m_cbuf_mtx.lock();

	if (m_cbuf_length > 0)
	{
		if (m_file == nullptr)
		{
			m_file = fopen(m_file_path.c_str(), "a");
			if (m_file == nullptr)
			{
				m_logger->error("[%s] std::fopen() failed", m_file_path.c_str());
			}
		}

		if (fprintf(m_file, "%s", m_cbuf) < 0)
		{
			m_logger->error("[%s] std::fprintf() failed: %s", m_file_path.c_str(), m_cbuf);
		}
		m_cbuf_length = 0;
	}
	delete[] m_cbuf;

	m_cbuf_mtx.unlock();

	if (m_file != nullptr)
	{
		if (fclose(m_file) != 0)
		{
			m_logger->error("[%s] std::fclose() failed", m_file_path.c_str());
		}
		m_file = nullptr;
	}
}

bool WriterImpl::create_buffer(void)
{
	try
	{
		m_cbuf = new char[m_cbuf_capacity];  // On failure, it throws a bad_alloc exception.
	}
	catch (const std::exception & e)
	{
		this->release_buffer();
		m_logger->error("[%s] %s: %i", m_file_path.c_str(), e.what(), m_cbuf_capacity);
		return false;
	}
	
	return true;
}

void WriterImpl::release_buffer(void)
{
	if (m_cbuf != nullptr)
	{
		delete m_cbuf;
		m_cbuf = nullptr;
		m_cbuf_length = 0;
		m_cbuf_state = BS_FREE;
	}
}

bool WriterImpl::printf(const char * format, ...)
{
	std::lock_guard<std::mutex> lockguard(m_cbuf_mtx);

	if (m_cbuf == nullptr) return false;

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
			char * cbuf;
			try
			{
				cbuf = new char[m_cbuf_capacity]; // On failure, it throws a bad_alloc exception.
			}
			catch (const std::exception & e)
			{
				this->release_buffer();
				m_logger->error("[%s] %s: %i", m_file_path.c_str(), e.what(), m_cbuf_capacity);
				return false;
			}
			m_logger->info("[%s] expand buffer: %i\n", m_file_path.c_str(), m_cbuf_capacity);
			std::strcpy(cbuf, m_cbuf);
			delete[] m_cbuf;
			m_cbuf = cbuf;
		}
	} while (true);

	return true;
}

BufferStateRet WriterImpl::flush(void)
{
	BufferStateRet ret;

	std::lock_guard<std::mutex> lockguard(m_cbuf_mtx);

	ret.former = m_cbuf_state;

	if (m_cbuf_length > 0 && m_file != nullptr)
	{
		if (fprintf(m_file, "%s", m_cbuf) != m_cbuf_length)
		{
			this->release_buffer();
			m_logger->error("[%s] std::fprintf() fail: %s", m_file_path.c_str(), m_cbuf);
		}
		else
		{
			m_cbuf[0] = '\0';
			m_cbuf_length = 0;
			m_cbuf_state = BS_FREE;
		}
	}

	ret.lastest = m_cbuf_state;

	return ret;
}

bool WriterImpl::is_file_open(void)
{
	return m_file != nullptr;
}

void WriterImpl::open_file(void)
{
	if (m_file == nullptr)
	{
		m_file = fopen(m_file_path.c_str(), "a");
		if (m_file == nullptr)
		{
			m_logger->error("[%s] std::fopen() fail", m_file_path.c_str());
			m_cbuf_mtx.lock();
			this->release_buffer();
			m_cbuf_mtx.unlock();
		}
	}
}

void WriterImpl::close_file(void)
{
	if (m_file != nullptr)
	{
		if (fclose(m_file) != 0)
		{
			m_logger->error("[%s] std::fclose() fail", m_file_path.c_str());
			m_cbuf_mtx.lock();
			this->release_buffer();
			m_cbuf_mtx.unlock();
		}
		m_file = nullptr;
	}
}

