
#include <cstdio>
#include <mutex>
#include <ctime>
#include <cstdarg>
#include <cstring>
#include "Logging.h"


class LoggingGuard
{
public:
	LoggingGuard(void)
	{
		flog = fopen("fatwlog.txt", "a");
		std::setvbuf(flog, nullptr, _IONBF, 0);
		fprintf(flog, "[%s]\n", __FUNCTION__);
	}

	~LoggingGuard(void)
	{
		fprintf(flog, "[%s]\n", __FUNCTION__);
		fclose(flog);
	}

	std::mutex log_locker;

	FILE * flog{ nullptr };
};

LoggingGuard logging_guard;


void glog(const char * format, ...)
{
	if (logging_guard.flog == nullptr)
	{
		fprintf(stderr, "%s\n", "Logging functionality is unavailable.");
		return;
	}

	std::lock_guard<std::mutex> lockguard(logging_guard.log_locker);

	char cbuf[BUFSIZ];
	auto t = std::time(nullptr);
	strftime(cbuf, BUFSIZ, "[%Y/%m/%d %H:%M:%S] ", std::localtime(&t));
	strcat(cbuf, format);

	va_list args;
	va_start(args, format);
	vfprintf(logging_guard.flog, cbuf, args);
	va_end(args);

	fflush(logging_guard.flog);

	return;
}
