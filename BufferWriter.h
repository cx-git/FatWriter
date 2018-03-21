#pragma once

#include <cstdio>
#include <mutex>
#include <string>

enum BufferState
{
	BS_FREE = 0,
	BS_FILL,
	BS_FULL,
	BS_EXPAND
};

struct BufferStateRet
{
	BufferState lastest;
	BufferState former;
};

class FormatWriter
{
public:
	virtual void printf(const char * format, ...) = 0;
};

class BufferWriter
	: public FormatWriter
{
public:
	explicit BufferWriter(const int capacity, const std::string & path);
	virtual ~BufferWriter(void);

	// [MultThread Safe] called by user thread
	virtual void printf(const char * format, ...);

	// [MultThread Safe] called by background thread
	BufferStateRet flush(void);

	// [MultThread Unsafe] called by background thread
	bool is_file_open(void);

	// [MultThread Unsafe] called by background thread
	void open_file(void);

	// [MultThread Unsafe] called by background thread 
	void close_file(void);

private:
	std::mutex m_cbuf_locker;
	int m_cbuf_length{ 0 };
	int m_cbuf_size;
	int m_cbuf_capacity;	
	BufferState m_cbuf_state{ BS_FREE };
	char * m_cbuf{ nullptr };

	const std::string m_file_path;
	FILE * m_file{ nullptr };
};
