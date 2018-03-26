#pragma once

#include <string>
#include <memory>

namespace fatwriter
{
	class Writer
	{
	public:
		virtual ~Writer(void) {};

		/*
		# Return false: exception happened, read hub's logfile. Writer is out of service.
		# Thread safe.
		*/
		virtual bool printf(const char * format, ...) = 0;
	};

	class WriterHub
	{
	public:
		virtual ~WriterHub(void) {};

		/*
		# Return nullptr: exception happened, read hub's logfile.
		# Thread safe.
		*/
		virtual Writer * create(const std::string & path) = 0;
	};

	/*
	# Return nullptr: exception happened, read error_prompt.
	# Thread safe.
	*/
	std::shared_ptr<WriterHub> create_writer_hub(
		int opening_files_limit,
		int buffer_capacity,
		int flush_interval_ms,
		std::string logger_path,
		std::string & error_prompt);

	/*
	# Not for user.
	*/
	class Logger
	{
	public:
		virtual void info(const char * format, ...) = 0;
		virtual void error(const char * format, ...) = 0;
	};
}

using FormatWriter = fatwriter::Writer;	// Forwards Compatibility
