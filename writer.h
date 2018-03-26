#pragma once

#include <mutex>
#include <string>

#include "fatwriter.h"
using namespace fatwriter;

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

class WriterImpl
	: public Writer
{
public:
	explicit WriterImpl(const int capacity, const std::string & path, Logger * logger);
	virtual ~WriterImpl(void);
	bool create_buffer(void);
	void release_buffer(void);
	virtual bool printf(const char * format, ...);
	BufferStateRet flush(void);
	bool is_file_open(void);
	void open_file(void);
	void close_file(void);

private:
	std::mutex m_cbuf_mtx;
	int m_cbuf_length{ 0 };
	int m_cbuf_size;
	int m_cbuf_capacity;
	const std::string m_file_path;
	BufferState m_cbuf_state{ BS_FREE };
	char * m_cbuf{ nullptr };	
	FILE * m_file{ nullptr };
	Logger * m_logger{ nullptr };
};
