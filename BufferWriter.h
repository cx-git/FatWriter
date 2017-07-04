#pragma once

#include <cstdio>
#include <mutex>
#include <string>
#include <atomic>

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
	BufferWriter(const int capacity, const std::string & path);
	virtual ~BufferWriter(void);

	std::string get_file_path(void);

	// [MultThread Safe] called by user thread
	virtual void printf(const char * format, ...);

	// [MultThread Safe] called by background thread
	BufferStateRet flush(void);

	// [MultThread Unsafe] called by background thread
	bool isopen(void);

	// [MultThread Unsafe] called by background thread
	int fsopen(void);

	// [MultThread Unsafe] called by background thread 
	int fsclose(void);

private:
	std::mutex m_cbuf_locker;
	int m_cbuf_length{ 0 };
	int m_cbuf_size;
	int m_cbuf_capacity;	
	char * m_cbuf_ptr{ nullptr };
	BufferState m_cbuf_state{ BS_FREE };

	std::string m_fs_path;
	FILE * m_fs_ptr{ nullptr };
};
